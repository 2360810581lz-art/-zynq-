---
name: vivado-report-gate
description: Screen Vivado timing coverage DRC and resource reports offline, rejecting missing metrics and negative global slack.
---

# 实现报告与约束覆盖筛查

确认四份报告来自同一布局布线结果，再运行 scripts/check_reports.py。全局setup/hold与局部域分别记录。缺字段和负余量均停止“通过”结论；保持full_signoff=false，另列CDC与板级验证。

使用场景、命令、验证记录与失效条件见 [README.md](README.md)。仅将其中“本项目实例”视作实例，其他题目应替换参数和输入。运行本技能不会扩大对实际设备操作的授权。
