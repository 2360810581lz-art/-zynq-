set source_dir [file dirname [file normalize [info script]]]
open_project color_hls
set_top live_object_color
set flags "-std=c++11 -I$source_dir"
if {[info exists ::env(COLOR_RTL_TEST)]} {append flags " -DCOLOR_RTL_TEST"}
add_files [file join $source_dir color.cpp] -cflags $flags
add_files -tb [file join $source_dir test_color.cpp] -cflags $flags
foreach path [glob [file join $source_dir *.bin]] {add_files -tb $path}
add_files -tb [file join $source_dir cases.txt]
open_solution solution1
set_part {xc7z020clg400-2}
create_clock -period 10.0 -name default
csim_design -clean
csynth_design
if {[info exists ::env(COLOR_RTL_TEST)]} {
    cosim_design -rtl verilog -tool xsim -trace_level none
    source [file join $source_dir hls_export_2019.tcl]
    export_ip_2019
}
puts "TOOLCHAIN_STATUS=SUCCESS"
exit
