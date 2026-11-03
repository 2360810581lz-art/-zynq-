---
name: hls-golden
description: Compare structured golden outputs and inspect actual HLS synthesis and RTL cosimulation reports, exposing inconsistent summary receipts.
---

# HLS黄金参考与报告核查

先确认独立oracle和帧协议，再运行 scripts/compare_json.py 与 scripts/inspect_hls.py。错误字段、空比对、NaN、布尔与数值混淆均不能算通过。收据冲突时保留原件，报告 actual RTL result 与 receipt conflict 两项，不覆盖历史收据。

使用场景、命令、验证记录与失效条件见 [README.md](README.md)。仅将其中“本项目实例”视作实例，其他题目应替换参数和输入。运行本技能不会扩大对实际设备操作的授权。
