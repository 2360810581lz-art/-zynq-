set_property PACKAGE_PIN U7 [get_ports {cam_gpio_tri_io[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {cam_gpio_tri_io[0]}]
set_property PULLUP true [get_ports {cam_gpio_tri_io[0]}]

set_property -dict {PACKAGE_PIN T5 IOSTANDARD LVCMOS33} [get_ports cam_i2c_scl_io]
set_property -dict {PACKAGE_PIN U5 IOSTANDARD LVCMOS33} [get_ports cam_i2c_sda_io]

set_property PULLUP true [get_ports cam_i2c_scl_io]
set_property PULLUP true [get_ports cam_i2c_sda_io]

set_property INTERNAL_VREF 0.6 [get_iobanks 13]

set_property -dict {PACKAGE_PIN W8 IOSTANDARD HSUL_12} [get_ports dphy_clk_lp_n]
set_property -dict {PACKAGE_PIN V8 IOSTANDARD HSUL_12} [get_ports dphy_clk_lp_p]

set_property -dict {PACKAGE_PIN U10 IOSTANDARD HSUL_12} [get_ports {dphy_data_lp_n[0]}]
set_property -dict {PACKAGE_PIN T9 IOSTANDARD HSUL_12} [get_ports {dphy_data_lp_p[0]}]
set_property -dict {PACKAGE_PIN Y13 IOSTANDARD HSUL_12} [get_ports {dphy_data_lp_n[1]}]
set_property -dict {PACKAGE_PIN Y12 IOSTANDARD HSUL_12} [get_ports {dphy_data_lp_p[1]}]

set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVDS_25} [get_ports dphy_hs_clock_clk_n]
set_property -dict {PACKAGE_PIN Y7 IOSTANDARD LVDS_25} [get_ports dphy_hs_clock_clk_p]


set_property -dict {PACKAGE_PIN Y11 IOSTANDARD LVDS_25} [get_ports {dphy_data_hs_n[0]}]
set_property -dict {PACKAGE_PIN W11 IOSTANDARD LVDS_25} [get_ports {dphy_data_hs_p[0]}]
set_property -dict {PACKAGE_PIN U8 IOSTANDARD LVDS_25} [get_ports {dphy_data_hs_n[1]}]
set_property -dict {PACKAGE_PIN U9 IOSTANDARD LVDS_25} [get_ports {dphy_data_hs_p[1]}]

# OV5640 cfg_702p_60fps in the active SDK declares MIPISCLK=280 MHz.
# This constraint applies to that fixed 720p configuration; confirm on hardware
# before certifying the camera interface or enabling a different sensor mode.
create_clock -name camera_hs -period 3.571429 [get_ports dphy_hs_clock_clk_p]

# CSI status counters cross via CSI_StatusSnapshot XPM handshake.
# XPM supplies its scoped CDC constraints; no direct counter false paths.
