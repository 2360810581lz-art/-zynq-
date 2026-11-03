# SDK 2019.1 only. A fresh workspace keeps the original camera SDK untouched.
if {$argc != 3} {error "Expected workspace, HDF, driver repository"}
set workspace [file normalize [lindex $argv 0]]
set hdf [file normalize [lindex $argv 1]]
set driver_repo [file normalize [lindex $argv 2]]
if {[catch {
    setws $workspace
    repo -set $driver_repo
    createhw -name hw_2019 -hwspec $hdf
    createbsp -name mipi_hdmi_bsp_2019 -hwproject hw_2019 -proc ps7_cortexa9_0 -os standalone
    configbsp -bsp mipi_hdmi_bsp_2019 stdin ps7_uart_1
    configbsp -bsp mipi_hdmi_bsp_2019 stdout ps7_uart_1
    regenbsp -bsp mipi_hdmi_bsp_2019
    projects -build -type bsp -name mipi_hdmi_bsp_2019
    createapp -name fsbl_2019 -app {Zynq FSBL} -hwproject hw_2019 -proc ps7_cortexa9_0 -os standalone -lang c
    configbsp -bsp fsbl_2019_bsp stdin ps7_uart_0
    configbsp -bsp fsbl_2019_bsp stdout ps7_uart_0
    regenbsp -bsp fsbl_2019_bsp
    projects -build -type bsp -name fsbl_2019_bsp
    projects -build -type app -name fsbl_2019
} message]} {
    puts stderr "TOOLCHAIN_STATUS=FAILED $message"
    exit 1
}
puts "TOOLCHAIN_STATUS=SUCCESS"
exit 0
