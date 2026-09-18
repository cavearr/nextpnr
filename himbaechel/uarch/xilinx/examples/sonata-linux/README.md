# Sonata LiteX Linux SoC, congestion-driven spreading (xc7a50tcsg324-2)

The lowRISC Sonata ONE's Linux SoC from
[sonata-linux](https://github.com/jrrk2/sonata-linux): LiteX with a
dual-core VexRiscv-SMP (RV32IMAFD, sv32 MMU, one FPU shared by both harts,
64 KiB I-caches, 16 KiB D-caches), privileged debug over a JTAG TAP,
8 MB of HyperRAM, SPI flash and an SD card.  Linux runs execute-in-place
from the flash.

Like the VC707 LiteX entries this ships a **netlist**, not Verilog:
`sonata.json.gz` is yosys' output (Yosys 0.63+173, `synth_xilinx -flatten
-abc9 -arch xc7`), gzipped from 59.4 MB to 2.9 MB.  The BIOS ROM is resolved
into RAMB `INIT` words at synthesis time, so the netlist needs no compiler.

| | |
|---|---|
| cells | 39 395 |
| SLICE_LUTX | 28 118 / 65 200 (43 %) |
| SLICE_FFX | 16 530 / 65 200 (25 %) |
| RAMB36E1 | 54 / 75 (72 %) |
| DSP48E1 | 21 / 120 (17 %) |

## Why it is in the gate

It is the design that needs `--placer-heap-congestion-spread`.

Without the flag HeAP packs the SoC into one corner of the die -- the
right-hand columns sit nearly empty -- and router2 never converges from
there: overuse falls to a floor of about 5 k wires by iteration 11 and then
climbs, still unrouted after 90 minutes (measured in the review of the
spreading prototype on openXC7/nextpnr#5, on an earlier base of this
branch).  With the flag the spreader
treats routing-congested tiles as having less room than they do, the
placement opens up, and router2 reaches zero overuse in well under 30
iterations.  Measured locally on this branch with the CI entry's flags:

    iter=1   overused=48805
    iter=2   overused=16696
    ...
    iter=17  overused=0         Routing complete, no hold violations

    main_crg_clkout_buf0   59.74 MHz  (PASS at 50 MHz)
    main_crg_clkout_buf1  276.01 MHz  (PASS at 100 MHz)
    main_jtag_tck_bufg    227.69 MHz  (PASS at 12.5 MHz)

HeAP 133 s, router2 2405 s, 45 minutes end to end on 12 threads.

The entry pins the flag so the path stays exercised.  It is `stretch`
tier -- non-blocking, not proved -- like the other Linux SoCs.

## What it has already caught

Spreading moves cells into places the default placement never used, and
the first full build of this netlist exposed two defects that had nothing
to do with the spreader:

- A falling-edge flip-flop (the netlist has one `FDRE_1` among 16 530 FFs)
  could share a half-slice with rising-edge ones, because the control-set
  check read `IS_CLK_INVERTED`, which no FF carries.  The FASM writer then
  aborted on `negedge_ff` in `write_ffs_config`.
- The HyperRAM data lines are LVCMOS18 inout pairs on an HR bank, and the
  FASM writer applied an HP-bank rule to them -- `.IN` on one half and
  `IBUF_HP_BANK_GLUE` on the other -- which prjxray has no key for on
  `RIOB33`.

## Hardware status

Not yet confirmed on a board.  The netlist is the one sonata-linux's
`make bitstream` synthesises; the bitstream built from it here has not
been flashed.

## Regenerating

In a sonata-linux checkout, `make bitstream` runs LiteX's openXC7 flow:

    python3 make.py --board=sonata --toolchain=openxc7 \
        --cpu-count=2 --with-privileged-debug --jtag-tap --wishbone-force-32b \
        --icache-size=65536 --icache-ways=16 --dcache-size=16384 --dcache-ways=4 \
        --with-fpu --cpu-per-fpu 2 --build

The netlist is `build/sonata/gateware/sonata.json` and the constraints
`sonata.xdc`; `--freq 25` in the CI entry matches the LiteX build script.
