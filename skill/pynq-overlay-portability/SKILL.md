---
name: pynq-overlay-portability
description: Prepare board independent PYNQ overlay metadata contracts and a staged smoke test plan for IP registers and DMA integration.
---

# 通用PYNQ Overlay迁移与元数据契约

保持算法无关，先读README和 references/pynq_runtime.md。运行 scripts/preflight.py 检查元数据契约。只在真实目标板与配套硬件产物上执行分阶段运行模板；无硬件证据时明确offline_metadata_only。

使用场景、命令、验证记录与失效条件见 [README.md](README.md)。仅将其中“本项目实例”视作实例，其他题目应替换参数和输入。运行本技能不会扩大对实际设备操作的授权。
