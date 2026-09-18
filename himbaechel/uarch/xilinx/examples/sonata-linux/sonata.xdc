################################################################################
# IO constraints
################################################################################
# clk25:0
set_property LOC P15 [get_ports {clk25}]
set_property IOSTANDARD LVCMOS33 [get_ports {clk25}]

# serial:0.tx
set_property LOC C17 [get_ports {serial_tx}]
set_property IOSTANDARD LVCMOS33 [get_ports {serial_tx}]

# serial:0.rx
set_property LOC D18 [get_ports {serial_rx}]
set_property IOSTANDARD LVCMOS33 [get_ports {serial_rx}]

# hyperram:0.dq
set_property LOC B1 [get_ports {hyperram0_dq[0]}]
set_property SLEW FAST [get_ports {hyperram0_dq[0]}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_dq[0]}]

# hyperram:0.dq
set_property LOC E2 [get_ports {hyperram0_dq[1]}]
set_property SLEW FAST [get_ports {hyperram0_dq[1]}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_dq[1]}]

# hyperram:0.dq
set_property LOC H1 [get_ports {hyperram0_dq[2]}]
set_property SLEW FAST [get_ports {hyperram0_dq[2]}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_dq[2]}]

# hyperram:0.dq
set_property LOC A1 [get_ports {hyperram0_dq[3]}]
set_property SLEW FAST [get_ports {hyperram0_dq[3]}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_dq[3]}]

# hyperram:0.dq
set_property LOC E1 [get_ports {hyperram0_dq[4]}]
set_property SLEW FAST [get_ports {hyperram0_dq[4]}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_dq[4]}]

# hyperram:0.dq
set_property LOC B2 [get_ports {hyperram0_dq[5]}]
set_property SLEW FAST [get_ports {hyperram0_dq[5]}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_dq[5]}]

# hyperram:0.dq
set_property LOC C1 [get_ports {hyperram0_dq[6]}]
set_property SLEW FAST [get_ports {hyperram0_dq[6]}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_dq[6]}]

# hyperram:0.dq
set_property LOC D2 [get_ports {hyperram0_dq[7]}]
set_property SLEW FAST [get_ports {hyperram0_dq[7]}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_dq[7]}]

# hyperram:0.rwds
set_property LOC F1 [get_ports {hyperram0_rwds}]
set_property SLEW FAST [get_ports {hyperram0_rwds}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_rwds}]

# hyperram:0.cs_n
set_property LOC J2 [get_ports {hyperram0_cs_n}]
set_property SLEW FAST [get_ports {hyperram0_cs_n}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_cs_n}]

# hyperram:0.rst_n
set_property LOC C2 [get_ports {hyperram0_rst_n}]
set_property SLEW FAST [get_ports {hyperram0_rst_n}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_rst_n}]

# hyperram:0.clk
set_property LOC H2 [get_ports {hyperram0_clk}]
set_property SLEW FAST [get_ports {hyperram0_clk}]
set_property IOSTANDARD LVCMOS18 [get_ports {hyperram0_clk}]

# spiflash4x:0.cs_n
set_property LOC D10 [get_ports {spiflash4x_cs_n}]
set_property IOSTANDARD LVCMOS33 [get_ports {spiflash4x_cs_n}]

# spiflash4x:0.clk
set_property LOC A8 [get_ports {spiflash4x_clk}]
set_property IOSTANDARD LVCMOS33 [get_ports {spiflash4x_clk}]

# spiflash4x:0.dq
set_property LOC C11 [get_ports {spiflash4x_dq[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {spiflash4x_dq[0]}]

# spiflash4x:0.dq
set_property LOC C10 [get_ports {spiflash4x_dq[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {spiflash4x_dq[1]}]

# spiflash4x:0.dq
set_property LOC A10 [get_ports {spiflash4x_dq[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {spiflash4x_dq[2]}]

# spiflash4x:0.dq
set_property LOC A9 [get_ports {spiflash4x_dq[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {spiflash4x_dq[3]}]

# spi_eth:0.clk
set_property LOC E3 [get_ports {spi_eth_clk}]
set_property IOSTANDARD LVCMOS18 [get_ports {spi_eth_clk}]

# spi_eth:0.mosi
set_property LOC D5 [get_ports {spi_eth_mosi}]
set_property IOSTANDARD LVCMOS18 [get_ports {spi_eth_mosi}]

# spi_eth:0.miso
set_property LOC D4 [get_ports {spi_eth_miso}]
set_property IOSTANDARD LVCMOS18 [get_ports {spi_eth_miso}]

# spi_eth:0.cs_n
set_property LOC H5 [get_ports {spi_eth_cs_n}]
set_property IOSTANDARD LVCMOS18 [get_ports {spi_eth_cs_n}]

# eth_rst_n:0
set_property LOC J5 [get_ports {eth_rst_n}]
set_property IOSTANDARD LVCMOS18 [get_ports {eth_rst_n}]

# jtag:0.tck
set_property LOC E15 [get_ports {jtag_tck}]
set_property IOSTANDARD LVCMOS33 [get_ports {jtag_tck}]

# jtag:0.tms
set_property LOC H15 [get_ports {jtag_tms}]
set_property IOSTANDARD LVCMOS33 [get_ports {jtag_tms}]

# jtag:0.tdi
set_property LOC G17 [get_ports {jtag_tdi}]
set_property IOSTANDARD LVCMOS33 [get_ports {jtag_tdi}]

# jtag:0.tdo
set_property LOC J14 [get_ports {jtag_tdo}]
set_property IOSTANDARD LVCMOS33 [get_ports {jtag_tdo}]

# user_led:0
set_property LOC B13 [get_ports {user_led0}]
set_property IOSTANDARD LVCMOS33 [get_ports {user_led0}]

# user_led:1
set_property LOC B14 [get_ports {user_led1}]
set_property IOSTANDARD LVCMOS33 [get_ports {user_led1}]

# user_led:2
set_property LOC C12 [get_ports {user_led2}]
set_property IOSTANDARD LVCMOS33 [get_ports {user_led2}]

# user_led:3
set_property LOC B12 [get_ports {user_led3}]
set_property IOSTANDARD LVCMOS33 [get_ports {user_led3}]

# user_led:4
set_property LOC B11 [get_ports {user_led4}]
set_property IOSTANDARD LVCMOS33 [get_ports {user_led4}]

# user_led:5
set_property LOC A11 [get_ports {user_led5}]
set_property IOSTANDARD LVCMOS33 [get_ports {user_led5}]

# user_led:6
set_property LOC F13 [get_ports {user_led6}]
set_property IOSTANDARD LVCMOS33 [get_ports {user_led6}]

# user_led:7
set_property LOC F14 [get_ports {user_led7}]
set_property IOSTANDARD LVCMOS33 [get_ports {user_led7}]

# sdcard:0.data
set_property LOC V4 [get_ports {sdcard_data[0]}]
set_property SLEW FAST [get_ports {sdcard_data[0]}]
set_property PULLUP True [get_ports {sdcard_data[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {sdcard_data[0]}]

# sdcard:0.data
set_property LOC R7 [get_ports {sdcard_data[1]}]
set_property SLEW FAST [get_ports {sdcard_data[1]}]
set_property PULLUP True [get_ports {sdcard_data[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {sdcard_data[1]}]

# sdcard:0.data
set_property LOC V5 [get_ports {sdcard_data[2]}]
set_property SLEW FAST [get_ports {sdcard_data[2]}]
set_property PULLUP True [get_ports {sdcard_data[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {sdcard_data[2]}]

# sdcard:0.data
set_property LOC T8 [get_ports {sdcard_data[3]}]
set_property SLEW FAST [get_ports {sdcard_data[3]}]
set_property PULLUP True [get_ports {sdcard_data[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {sdcard_data[3]}]

# sdcard:0.cmd
set_property LOC R8 [get_ports {sdcard_cmd}]
set_property SLEW FAST [get_ports {sdcard_cmd}]
set_property PULLUP True [get_ports {sdcard_cmd}]
set_property IOSTANDARD LVCMOS33 [get_ports {sdcard_cmd}]

# sdcard:0.clk
set_property LOC U6 [get_ports {sdcard_clk}]
set_property SLEW FAST [get_ports {sdcard_clk}]
set_property IOSTANDARD LVCMOS33 [get_ports {sdcard_clk}]

# sdcard:0.cd
set_property LOC B3 [get_ports {sdcard_cd}]
set_property SLEW FAST [get_ports {sdcard_cd}]
set_property IOSTANDARD LVCMOS18 [get_ports {sdcard_cd}]

# user_sw:0
set_property LOC D3 [get_ports {user_sw0}]
set_property IOSTANDARD LVCMOS18 [get_ports {user_sw0}]
set_property PULLUP True [get_ports {user_sw0}]

# user_sw:1
set_property LOC F4 [get_ports {user_sw1}]
set_property IOSTANDARD LVCMOS18 [get_ports {user_sw1}]
set_property PULLUP True [get_ports {user_sw1}]

# user_sw:2
set_property LOC F3 [get_ports {user_sw2}]
set_property IOSTANDARD LVCMOS18 [get_ports {user_sw2}]
set_property PULLUP True [get_ports {user_sw2}]

################################################################################
# Design constraints
################################################################################

create_clock -name jtag_tck -period 80.0 [get_ports {jtag_tck}]

set_output_delay -clock jtag_tck -max 5.0 [get_ports {jtag_tdo}]

set_output_delay -clock jtag_tck -min 0.0 [get_ports {jtag_tdo}]
################################################################################
# Clock constraints
################################################################################

create_clock -name clk25 -period 40.0 [get_ports clk25]