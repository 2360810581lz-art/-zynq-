# Vivado 2019.1. Always build in a NEW directory, never in frozen sources.
if {$argc != 2} {error "Expected output directory and action create|pl"}
set package_root [file dirname [file dirname [file normalize [info script]]]]
set output_dir [file normalize [lindex $argv 0]]
set action [lindex $argv 1]
if {$action ni {create pl}} {error "action must be create or pl"}
if {[file exists [file join $output_dir project]]} {error "Output project already exists"}
file mkdir $output_dir
proc collect_components {dir} {
    set out {}
    if {[file exists [file join $dir component.xml]]} {lappend out $dir}
    foreach d [glob -nocomplain -type d -directory $dir *] {set out [concat $out [collect_components $d]]}
    return $out
}
if {[catch {
    create_project autonomous_sorting [file join $output_dir project] -part xc7z020clg400-2
    set_property target_language Verilog [current_project]
    set_property ip_repo_paths [collect_components [file join $package_root src ip]] [current_project]
    update_ip_catalog
    add_files [glob [file join $package_root src rtl *.v]]
    add_files [glob [file join $package_root src rtl control *.v]]
    add_files [glob [file join $package_root src rtl control *.vh]]
    set_property include_dirs [list [file join $package_root src rtl control]] [get_filesets sources_1]
    file mkdir [file join $output_dir bd]
    set bd_file [file join $output_dir bd system.bd]
    file copy [file join $package_root src bd system.bd] $bd_file
    add_files $bd_file
    add_files -fileset constrs_1 [glob [file join $package_root src constraints *.xdc]]
    foreach xdc [get_files -quiet *mipi*.xdc] {set_property PROCESSING_ORDER EARLY $xdc}
    set_property top camera_claw_top [get_filesets sources_1]
    update_compile_order -fileset sources_1
    open_bd_design $bd_file
    validate_bd_design
    save_bd_design
    report_ip_status -file [file join $output_dir ip_status.rpt]
    set locked {}
    foreach ip [get_ips -quiet] {if {[get_property IS_LOCKED $ip]} {lappend locked $ip}}
    if {[llength $locked]} {error "Locked IP: $locked"}
    if {$action eq "pl"} {
        generate_target all [get_files $bd_file]
        launch_runs synth_1 -jobs 4
        wait_on_run synth_1
        if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Synthesis incomplete"}
        launch_runs impl_1 -to_step write_bitstream -jobs 4
        wait_on_run impl_1
        if {[get_property PROGRESS [get_runs impl_1]] ne "100%"} {error "Implementation incomplete"}
        open_run impl_1
        report_timing_summary -report_unconstrained -file [file join $output_dir timing.rpt]
        report_utilization -file [file join $output_dir utilization.rpt]
        check_timing -verbose -file [file join $output_dir check_timing.rpt]
        report_drc -file [file join $output_dir drc.rpt]
        report_cdc -details -file [file join $output_dir cdc.rpt]
        set bit [file join $output_dir project autonomous_sorting.runs impl_1 camera_claw_top.bit]
        file copy $bit [file join $output_dir camera_claw_top.bit]
        write_checkpoint -force [file join $output_dir camera_claw_top_routed.dcp]
        write_hwdef -force -file [file join $output_dir camera_claw_top.hwdef]
        write_sysdef -force -hwdef [file join $output_dir camera_claw_top.hwdef] -bit [file join $output_dir camera_claw_top.bit] -file [file join $output_dir camera_claw_top.hdf]
    }
    close_project
} message]} {puts stderr "TOOLCHAIN_STATUS=FAILED $message"; exit 1}
puts "TOOLCHAIN_STATUS=SUCCESS"
exit 0
