/*
 *  nextpnr -- Next Generation Place and Route
 *
 *  Hold-time fixing by feedthrough-LUT insertion (xc7).
 *
 *  A measured interconnect delay model lets the placer pack timing paths
 *  tighter (better setup/fmax), but nothing in the flow lengthens a path that
 *  is now TOO SHORT: a flop-to-flop or flop-to-BRAM-data arc whose min delay
 *  falls below the sink's hold requirement.  The timing engine already reports
 *  these (`ctx->timing_result.min_delay_violations`); on a flow that does not
 *  pass --timing-allow-fail they are fatal.  Vivado's phys_opt and OpenROAD's
 *  rsz repair_hold both fix this the same way: insert delay on the short path.
 *
 *  Here that delay is a feedthrough LUT -- a 6-LUT wired as an identity buffer
 *  (O6 = A1) spliced between the driver and the one violating sink.  It adds
 *  the LUT's own delay plus the routing to and from it; a sink that still
 *  violates after one buffer gets another on the next pass (the buffers chain,
 *  because each pass re-runs timing and re-targets the net now feeding the
 *  sink).  Only the violating sink is rerouted through the buffer; the net's
 *  other sinks are untouched.
 *
 *  The pass runs AFTER routing (in postRoute, before FASM): route -> analyse
 *  hold -> insert+place buffers -> reroute (router2 keeps already-routed arcs
 *  and routes only the new ones) -> re-analyse, up to a bounded number of
 *  passes.  Opt-in via --xilinx-hold-fix (or settings xilinx/holdFix).
 */

#include "log.h"
#include "nextpnr.h"
#include "router2.h"
#include "timing.h"
#include "util.h"

#include "extra_data.h"
#include "himbaechel_helpers.h"
#include "xilinx.h"

#define HIMBAECHEL_CONSTIDS "uarch/xilinx/constids.inc"
#include "himbaechel_constids.h"

NEXTPNR_NAMESPACE_BEGIN

namespace {
// Identity 6-LUT: O6(a5..a0) = a0 = A1.  INIT[i] set for odd i -> 0xAAAA...
constexpr int64_t LUT_BUF_INIT = int64_t(0xAAAAAAAAAAAAAAAAULL);
constexpr int DEFAULT_MAX_PASSES = 8;
// How far out (in tiles) to look for a free LUT bel to host a buffer.
constexpr int PLACE_SEARCH_RADIUS = 12;
} // namespace

// The sink of a hold path we want to lengthen: the last ROUTING segment of the
// violation report names the net feeding the endpoint and the endpoint pin.
struct HoldTarget
{
    IdString net;
    IdString sink_cell;
    IdString sink_port;
    bool operator==(const HoldTarget &o) const
    {
        return net == o.net && sink_cell == o.sink_cell && sink_port == o.sink_port;
    }
};

// Try to place `buf` at a free SLICE_LUTX 6-LUT bel, searching outward from
// `origin`.  Binds and validates each candidate; a lone combinational LUT is
// legal in any otherwise-free 6-LUT bel, so this normally succeeds on the
// first ring.  Returns the chosen bel, or BelId() if none validated.
static BelId place_hold_buffer(XilinxImpl *impl, Context *ctx, CellInfo *buf, Loc origin)
{
    auto try_tile = [&](int x, int y) -> BelId {
        if (x < 0 || y < 0 || x >= ctx->getGridDimX() || y >= ctx->getGridDimY())
            return BelId();
        for (BelId bel : ctx->getBelsByTile(x, y)) {
            if (ctx->getBelType(bel) != id_SLICE_LUTX)
                continue;
            Loc l = ctx->getBelLocation(bel);
            if ((l.z & 0xF) != BEL_6LUT)
                continue;
            if (!ctx->checkBelAvail(bel))
                continue;
            // The 6-LUT and its sibling 5-LUT at the same slice position share
            // the physical input pins (A1-A6).  A cell in the 5-LUT with a
            // different net on those pins collides with our A1 -- surfacing as
            // "attempting to reserve sink input path wire ... for nets X and Y"
            // at route setup.  Require the 5-LUT free too.
            BelId sib = ctx->getBelByLocation(Loc(l.x, l.y, (l.z & ~0xF) | BEL_5LUT));
            if (sib != BelId() && !ctx->checkBelAvail(sib))
                continue;
            ctx->bindBel(bel, buf, STRENGTH_STRONG);
            if (impl->isBelLocationValid(bel))
                return bel;
            ctx->unbindBel(bel);
        }
        return BelId();
    };

    for (int r = 0; r <= PLACE_SEARCH_RADIUS; r++) {
        if (r == 0) {
            BelId b = try_tile(origin.x, origin.y);
            if (b != BelId())
                return b;
            continue;
        }
        for (int dx = -r; dx <= r; dx++) {
            int dy = r - std::abs(dx);
            BelId b = try_tile(origin.x + dx, origin.y + dy);
            if (b != BelId())
                return b;
            if (dy != 0) {
                b = try_tile(origin.x + dx, origin.y - dy);
                if (b != BelId())
                    return b;
            }
        }
    }
    return BelId();
}

void XilinxImpl::fixup_hold()
{
    const ArchArgs &args = ctx->args;
    if (!args.options.count("hold-fix"))
        return;
    int max_passes = DEFAULT_MAX_PASSES;
    {
        std::string v = args.options["hold-fix"].as<std::string>();
        if (!v.empty()) {
            try {
                max_passes = std::stoi(v);
            } catch (...) {
                max_passes = DEFAULT_MAX_PASSES;
            }
        }
    }

    if (getenv("HOLDFIX_PROBE")) {
        // Does device wire iteration itself crash here, before any 2nd router2?
        // If so the corruption predates this pass entirely.
        long n = 0;
        for (auto w : ctx->getWires()) {
            (void)w;
            n++;
        }
        log_info("Hold-fix PROBE: iterated %ld wires OK.\n", n);
        return;
    }

    Router2Cfg rcfg(ctx);
    configureRouter2(rcfg);

    // Bisection aid: reroute the already-routed design without inserting
    // anything, to separate a router2 re-entrancy problem from an insertion bug.
    if (getenv("HOLDFIX_REROUTE_ONLY")) {
        log_info("Hold-fix: HOLDFIX_REROUTE_ONLY set -- rerouting with no insertion.\n");
        router2(ctx, rcfg);
        timing_analysis(ctx, true, true, false, true, true);
        return;
    }

    int total_buffers = 0;
    for (int pass = 0; pass < max_passes; pass++) {
        const auto &violations = ctx->timing_result.min_delay_violations;
        if (violations.empty()) {
            if (pass == 0)
                log_info("Hold-fix: no hold violations to fix.\n");
            break;
        }

        // Collect the unique sinks to lengthen this pass.
        std::vector<HoldTarget> targets;
        pool<std::string> seen;
        for (const auto &cp : violations) {
            const CriticalPath::Segment *last_routing = nullptr;
            for (const auto &seg : cp.segments)
                if (seg.type == CriticalPath::Segment::Type::ROUTING)
                    last_routing = &seg;
            if (!last_routing)
                continue;
            HoldTarget t{last_routing->net, last_routing->to.first, last_routing->to.second};
            std::string key = t.sink_cell.str(ctx) + "/" + t.sink_port.str(ctx);
            if (seen.count(key))
                continue;
            seen.insert(key);
            targets.push_back(t);
        }
        if (targets.empty())
            break;

        // 1. Create identity-buffer cells and splice them onto the violating
        //    sink arcs.  No bel binding yet -- tags must be (re)assigned first.
        struct Pending
        {
            CellInfo *buf;
            IdString sink_cell;
            Loc origin;
        };
        std::vector<Pending> pending;
        pool<IdString> touched_nets;
        for (const auto &t : targets) {
            if (!ctx->nets.count(t.net))
                continue;
            NetInfo *net = ctx->nets.at(t.net).get();
            if (net->driver.cell == nullptr)
                continue;
            if (!ctx->cells.count(t.sink_cell))
                continue;
            CellInfo *sink = ctx->cells.at(t.sink_cell).get();
            if (sink->bel == BelId())
                continue;
            if (sink->getPort(t.sink_port) != net) // stale report vs current netlist
                continue;

            NetInfo *buf_out = ctx->createNet(ctx->idf("%s$holdbuf%d$net", net->name.c_str(ctx), total_buffers));
            CellInfo *buf = ctx->createCell(ctx->idf("%s$holdbuf%d", net->name.c_str(ctx), total_buffers), id_SLICE_LUTX);
            buf->addInput(id_A1);
            buf->addOutput(id_O6);
            buf->params[id_INIT] = Property(LUT_BUF_INIT, 64);
            buf->connectPort(id_A1, net);       // buffer input  = original net
            buf->connectPort(id_O6, buf_out);   // buffer output = new net

            // Move just this sink from the original net onto the buffer output.
            sink->disconnectPort(t.sink_port);
            sink->connectPort(t.sink_port, buf_out);

            pending.push_back({buf, t.sink_cell, ctx->getBelLocation(sink->bel)});
            touched_nets.insert(net->name);
            total_buffers++;
        }
        if (pending.empty())
            break;

        // 2. Reindex and re-tag so get_tags() (used by bindBel -> notifyBelChange
        //    and by isBelLocationValid) sees the new cells.
        ctx->assignArchInfo();
        assign_cell_tags();

        // 3. Place each buffer near its sink.  On the rare failure, undo the
        //    splice so the netlist stays consistent.
        int placed = 0, failed = 0;
        for (auto &p : pending) {
            BelId bel = place_hold_buffer(this, ctx, p.buf, p.origin);
            if (bel != BelId()) {
                placed++;
                continue;
            }
            failed++;
            // undo: reconnect the sink to the original input net, drop the buffer
            NetInfo *orig = p.buf->getPort(id_A1);
            NetInfo *buf_out = p.buf->getPort(id_O6);
            CellInfo *sink = ctx->cells.at(p.sink_cell).get();
            IdString sink_port;
            for (auto &pr : sink->ports)
                if (pr.second.net == buf_out)
                    sink_port = pr.first;
            p.buf->disconnectPort(id_A1);
            p.buf->disconnectPort(id_O6);
            if (sink_port != IdString()) {
                sink->disconnectPort(sink_port);
                sink->connectPort(sink_port, orig);
            }
            ctx->cells.erase(p.buf->name);
            if (buf_out)
                ctx->nets.erase(buf_out->name);
        }
        if (failed)
            log_warning("Hold-fix pass %d: %d buffer(s) had no free LUT bel nearby and were skipped.\n", pass, failed);
        if (placed == 0)
            break;

        // 4. Rip up ALL routing so the reroute negotiates congestion globally.
        //    Ripping only the touched nets deadlocks: router2 keeps every
        //    already-routed arc (check_arc_routing -> skip) and never rips it,
        //    so a newly inserted buffer arc that collides with a pre-routed net
        //    can never displace it -- overuse floors at a few wires and the
        //    router spins forever.  A full rip-up makes the second router2 a
        //    fresh negotiated route, like the first, which converges.
        (void)touched_nets;
        for (auto &net_pair : ctx->nets) {
            NetInfo *net = net_pair.second.get();
            std::vector<WireId> wires;
            wires.reserve(net->wires.size());
            for (auto &w : net->wires)
                wires.push_back(w.first);
            for (WireId w : wires)
                ctx->unbindWire(w);
        }

        // 5. Reroute (incremental: already-routed arcs are kept) and re-analyse.
        //    Reindex+retag after any undo erasures so get_tags() stays valid for
        //    the router and for the finalisation that follows this pass.
        ctx->assignArchInfo();
        assign_cell_tags();
        log_info("Hold-fix pass %d: %d buffer(s) placed, rerouting %zu net(s)...\n", pass, placed,
                 touched_nets.size());
        router2(ctx, rcfg);
        timing_analysis(ctx, true /*slack_histogram*/, true /*print_fmax*/, false /*print_path*/,
                        true /*warn_on_failure*/, true /*update_results*/);
        log_info("Hold-fix pass %d: inserted %d feedthrough buffer(s); %zu hold violation(s) remain.\n", pass, placed,
                 ctx->timing_result.min_delay_violations.size());
    }

    if (total_buffers > 0)
        log_info("Hold-fix: inserted %d feedthrough buffer(s) total; %zu hold violation(s) remain.\n", total_buffers,
                 ctx->timing_result.min_delay_violations.size());
}

NEXTPNR_NAMESPACE_END
