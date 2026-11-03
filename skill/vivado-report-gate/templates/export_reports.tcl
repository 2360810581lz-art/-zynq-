# 在已经完成布局布线的Vivado工程中人工执行。不会启动实现或改变约束。
# 在2019.1英文报告上核验；执行前确认current_design正是待审版本。
set report_dir [file normalize ./review_reports]
file mkdir $report_dir
report_timing_summary -file [file join $report_dir timing.rpt]
check_timing -verbose -file [file join $report_dir check_timing.rpt]
report_drc -file [file join $report_dir drc.rpt]
report_utilization -file [file join $report_dir utilization.rpt]
# CDC及例外约束仍需单独审查，本脚本不声称完整签核。
