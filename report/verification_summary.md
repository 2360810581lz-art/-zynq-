# 交付检查记录

本次整理只做离线检查，没有重新运行 Vivado、SDK、HLS、XSim 或板卡。工程中的综合、仿真和板测原始记录保留各自的版本和日期。

| 检查 | 结果 | 收据 |
| --- | --- | --- |
| 技能行为测试 | 48 项通过，无失败或跳过 | sim/skill_validation/offline_checks.json |
| 示例与真实报告回放 | 7 组符合预期，含旧收据冲突和负时序两组拒绝案例 | sim/skill_validation/replay_log.json |
| 证据与协作记录 | 22 份证据、9 段旧摘录、77 条真实消息的索引哈希通过；26 项复盘 | sim/skill_validation/evidence_integrity.json |
| 构建入口 | 16 项离线检查通过；覆盖语法、CLI、配套脚本生成和启动器错误处理 | sim/skill_validation/build_entry_checks.json |
| 冻结机械仿真 | 三版本各 354 条断言、5943 个轨迹事件，输入哈希与轨迹一致 | sim/skill_validation/calibration_traces.json |
| 机械轨迹反例 | 在包外副本中改变一字节电机报文，校验按预期拒绝 | sim/skill_validation/calibration_negative.json |
| 文档排版 | 设计报告 16 页、协作记录 28 页，已渲染并逐页检查 | report 下的最终 PDF 和 Word |

完整性检查使用 `python -B sim/verify_package.py`，核对清单、英文命名、IP 依赖、PS 源文件、控制标定锁、HDF 内 bit、发布镜像配套、HLS 测试输入和 README 相对链接。先核验原包，再运行会更新收据的回放。

设计报告已列出当前时序负余量、HLS 颜色候选尚未整机集成、自主闭环尚待端到端验收、恢复后仍需冷启动和抓放回归等限制。离线检查不能替代这些验收。
