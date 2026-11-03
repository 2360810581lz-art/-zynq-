---
name: vision-motion-contract
description: Preview vision to actuator coordinate transforms and class routing offline with frame ownership freshness and range checks.
---

# 视觉到执行机构的任务契约

先定义坐标系与帧所有权，使用 examples/config.json 和 examples/frame.json 验证。运行 scripts/preview_task.py，失败则不给候选。下游遵守一次取一个、完整完成应答后刷新帧；查询应答不能作为运动完成。

使用场景、命令、验证记录与失效条件见 [README.md](README.md)。仅将其中“本项目实例”视作实例，其他题目应替换参数和输入。运行本技能不会扩大对实际设备操作的授权。
