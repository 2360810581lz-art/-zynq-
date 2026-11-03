# 01 PS PL 划分与存储预算

## 适用场景

准备把视觉、DSP 或数值算法迁移到 Zynq 等 CPU+FPGA 平台时使用。输入是算子步骤、尺寸、位宽、缓冲生命周期、处理器耗时、目标资源和带宽。

## 使用方法

用 `prompts/partition.md` 整理依赖图，再填 `templates/partition.csv`。同时存活的权重、特征图、行缓存和接口缓冲一起预算；共享物理存储只算一次。优先选可独立比对的核，再做整合实现。

## 提炼来源与已验证效果

E19 的容量复算纠正了 D02 的概括：140 个 BRAM36 理想容量为 645120 字节，409600 字节特征图单独可容纳，2937564 字节 INT8 权重约为总量 4.55 倍。E02 中十一核 LUT 简单相加已超器件预算。这两次观察形成了按生命周期预算的做法。

## 失效条件

缺少并发存储关系、可用带宽和系统保留量时，只能得到候选划分。换器件、位宽或 DDR 策略必须重算，理想物理容量不能直接当连续可用缓存。

测试定义在 `../tests/test_skills.py`，统一收据在包根 `sim/skill_validation/`。E 编号见 `../evidence/index.json`，D 编号见包根 `report/collaboration/records/dialogue_excerpts.jsonl`。
