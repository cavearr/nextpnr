#!/usr/bin/env python3
"""LVS check for any design the gate builds, not just vc707-johnson.

Extract a netlist back out of a design's FASM with fasm2netlist, and check it
against nextpnr's own placement dump for the SAME run (`-o placement=`).  The
dump names, for every SLICE LUT/FF nextpnr placed, the exact (tile, site,
bel) it landed on -- ground truth produced independently of fasm2netlist's
reconstruction.

Why this and not a frames hash: two FASMs can differ and both be right.
Placement is not canonical, the writer's feature order is not canonical, and
a routing change moves bytes without changing logic.  A hash detects drift; it
cannot tell drift from damage.  This can, because every assertion is
RELATIONAL -- each number is compared against another number from the same
run, never against a constant recorded from an earlier one.  A different
placement is not a failure; failing to account for the placement you were
given is.

The naming rules come from the fasm2netlist checkout itself (its LVS test is
the reference implementation), so the two cannot drift apart silently.
"""

import argparse
import importlib.util
import os
import re
import subprocess
import sys
import tempfile

SKIP = 77


def skip(msg):
    print("SKIP: %s" % msg)
    sys.exit(SKIP)


def load_reference(f2n_dir):
    """Borrow load_placements/parse_gate_netlist/expected_*_name from the
    fasm2netlist checkout so naming has exactly one definition."""
    ref = os.path.join(f2n_dir, "tests", "lvs", "test_johnson_lvs.py")
    if not os.path.exists(ref):
        skip("fasm2netlist LVS reference not found at %s" % ref)
    spec = importlib.util.spec_from_file_location("f2n_lvs_ref", ref)
    mod = importlib.util.module_from_spec(spec)
    # its main() is guarded by __name__, so importing runs no test
    spec.loader.exec_module(mod)
    return mod


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--exe", required=True, help="built fasm2netlist binary")
    ap.add_argument("--f2n-dir", required=True, help="fasm2netlist checkout (for its naming reference)")
    ap.add_argument("--fasm", required=True)
    ap.add_argument("--placement", required=True, help="nextpnr -o placement= dump from the same run")
    ap.add_argument("--db", required=True, help="prjxray-db checkout")
    ap.add_argument("--family", default="virtex7")
    ap.add_argument("--device", default="xc7vx485t")
    ap.add_argument("--design", default="design")
    args = ap.parse_args()

    for path, what in ((args.exe, "fasm2netlist binary"), (args.fasm, "FASM"),
                       (args.placement, "placement dump"), (args.db, "prjxray-db")):
        if not os.path.exists(path):
            skip("%s not found at %s" % (what, path))

    ref = load_reference(args.f2n_dir)

    ffs, luts = ref.load_placements(args.placement)
    if not ffs and not luts:
        # Legitimate for a design with no SLICE logic at all; nothing to check.
        skip("%s: placement dump names no SLICE FF or LUT cells" % args.design)

    with tempfile.TemporaryDirectory() as tmp:
        out_v = os.path.join(tmp, "%s_gates.v" % args.design)
        proc = subprocess.run(
            [args.exe, "--fasm", args.fasm, "--db", args.db, "--family", args.family,
             "--device", args.device, "--out", out_v, "--module", "%s_lvs" % args.design],
            capture_output=True, text=True)
        sys.stdout.write(proc.stdout)
        sys.stderr.write(proc.stderr)
        if proc.returncode != 0:
            print("::error::fasm2netlist exited %d on %s" % (proc.returncode, args.design))
            return 1

        m = re.search(r"(\d+) LUT6_2, (\d+) FF", proc.stderr)
        if not m:
            print("::error::no 'N LUT6_2, M FF' summary from fasm2netlist")
            return 1
        reported_lut, reported_ff = int(m.group(1)), int(m.group(2))
        ff_names, lut_names = ref.parse_gate_netlist(out_v)

    failures = []

    # ---- counts, all relational ----
    if not (reported_ff == len(ffs) == len(ff_names)):
        failures.append("FF count: fasm2netlist reported %d, emitted %d, nextpnr placed %d"
                        % (reported_ff, len(ff_names), len(ffs)))
    if reported_lut != len(lut_names):
        failures.append("LUT6_2 count: reported %d but emitted %d named cells"
                        % (reported_lut, len(lut_names)))
    # A superset is expected: a bitstream-configured LUT with no placed logical
    # cell is a route-through, and it is still real silicon.
    if reported_lut < len(luts):
        failures.append("fasm2netlist emitted fewer LUT6_2 (%d) than nextpnr placed columns (%d)"
                        " -- placed logic went missing" % (reported_lut, len(luts)))

    # ---- every placed cell must be recoverable by name ----
    missing_ff = [(t, s, c, i5, ref.expected_ff_name(t, s, c, i5))
                  for (t, s, c, i5) in ffs if ref.expected_ff_name(t, s, c, i5) not in ff_names]
    if missing_ff:
        failures.append("%d FF placed by nextpnr but not name-matched, e.g. %s"
                        % (len(missing_ff), missing_ff[0][4]))
    missing_lut = [(t, s, c, ref.expected_lut_name(t, s, c))
                   for (t, s, c) in luts if ref.expected_lut_name(t, s, c) not in lut_names]
    if missing_lut:
        failures.append("%d LUT columns placed by nextpnr but not name-matched, e.g. %s"
                        % (len(missing_lut), missing_lut[0][3]))

    print("%s: %d FF (placed %d), %d LUT6_2 (%d placed columns + %d route-throughs)"
          % (args.design, reported_ff, len(ffs), reported_lut, len(luts), reported_lut - len(luts)))

    if failures:
        for f in failures:
            print("::error::%s: %s" % (args.design, f))
        return 1
    print("LVS OK: %s -- every placed FF and LUT column accounted for in the extraction" % args.design)
    return 0


if __name__ == "__main__":
    sys.exit(main())
