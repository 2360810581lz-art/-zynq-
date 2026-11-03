set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

# Sole camera DDR input model in the isolated build (fixed IP example excluded).
# The pin clock and 15%-85% UI assumptions still need hardware confirmation.
# mipi.xdc defines the active camera root clock in the EARLY constraint group.
set camera_ui [expr {[get_property PERIOD [get_clocks camera_hs]] / 2.0}]
set camera_data [get_ports {dphy_data_hs_n[*] dphy_data_hs_p[*]}]
set_input_delay -clock camera_hs -min [expr {0.15 * $camera_ui}] $camera_data
set_input_delay -clock camera_hs -max [expr {0.85 * $camera_ui}] $camera_data
set_input_delay -clock camera_hs -clock_fall -min -add_delay [expr {0.15 * $camera_ui}] $camera_data
set_input_delay -clock camera_hs -clock_fall -max -add_delay [expr {0.85 * $camera_ui}] $camera_data
