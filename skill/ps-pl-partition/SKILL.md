---
name: ps-pl-partition
description: Derive a PS PL partition from measured workload, memory lifetime and interface constraints before migrating an algorithm to HLS.
---

# 软硬件划分与容量预算

读取算法与器件参数，运行 prompts/partition.md 工作流并填表。逐项复算位宽与字节数。输出PS保留项、PL候选项、接口代价和待测项；没有测量数据时不输出加速倍数。资源预算不满足时缩小候选或改变存储安排。

使用场景、命令、验证记录与失效条件见 [README.md](README.md)。仅将其中“本项目实例”视作实例，其他题目应替换参数和输入。运行本技能不会扩大对实际设备操作的授权。
