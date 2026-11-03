# Run from an empty output folder, with -tclargs object_color|feature.
if {$argc != 1} {error "Expected object_color or feature"}
set root [file dirname [file dirname [file normalize [info script]]]]
set kernel [lindex $argv 0]
if {$kernel ni {object_color feature}} {error "Unknown kernel"}
if {[file exists [file join [pwd] inputs]]} {error "Use a fresh output directory"}
file copy [file join $root src hls $kernel] [file join [pwd] inputs]
if {$kernel eq "feature"} {file copy [file join $root data feature_ppm] [file join [pwd] inputs testdata]}
source [file join [pwd] inputs run.tcl]
