# Keep the 2019.1 timestamp overflow workaround local to generated IP products.
# Run under the task runner's existing native-tool lock; never change host time.
proc export_ip_2019 {} {
    if {![catch {export_design -format ip_catalog} problem]} {return}
    set scripts [glob -nocomplain */solution1/impl/ip/run_ippack.tcl]
    if {[llength $scripts] != 1} {error $problem}
    set script [file normalize [lindex $scripts 0]]
    set log_path [file join [file dirname $script] vivado.log]
    if {![file exists $log_path]} {error $problem}
    set handle [open $log_path r]; set log [read $handle]; close $handle
    if {![string match {*bad lexical cast*} $log] || ![string match {*core_revision*} $log]} {
        error $problem
    }
    set handle [open $script r]; set body [read $handle]; close $handle
    set revision [clock seconds]
    if {$revision > 2147483647} {error "IP revision exceeds signed 32-bit range"}
    if {[regsub {set Revision\s+"[0-9]+"} $body "set Revision \"$revision\"" patched] != 1} {
        error "No generated revision found; original export error: $problem"
    }
    set compatible [file join [file dirname $script] run_ippack_2019.tcl]
    set handle [open $compatible w]; puts $handle $patched; close $handle
    if {![info exists ::env(VISION_VIVADO_EXECUTABLE)]} {error "Missing configured Vivado executable"}
    set previous [pwd]
    cd [file dirname $script]
    set rc [catch {exec $::env(VISION_VIVADO_EXECUTABLE) -mode batch -notrace -nojournal -source $compatible 2>@1} output]
    cd $previous
    puts $output
    if {$rc || [regexp {ERROR:} $output]} {error "2019.1 compatibility packaging failed"}
    puts "IP_REVISION_COMPAT_PASS revision=$revision"
}
