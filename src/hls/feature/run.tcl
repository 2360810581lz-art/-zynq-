set source_dir [file dirname [file normalize [info script]]]
open_project vision_feature_accel_hls
set_top vision_feature_accel
add_files [file join $source_dir src vision_feature_accel.cpp] -cflags "-I[file join $source_dir src]"
add_files -tb [file join $source_dir tb test_vision_feature_accel.cpp] -cflags "-I[file join $source_dir src]"
foreach sample [glob -directory [file join $source_dir testdata] *.ppm] {
    add_files -tb $sample
}
open_solution solution1
set_part {xc7z020clg400-2}
# Schedule with margin for routing at the actual 7 ns PS FCLK1 period.
create_clock -period 5.0 -name default
csim_design -clean
csynth_design
puts "TOOLCHAIN_STATUS=SUCCESS"
exit
