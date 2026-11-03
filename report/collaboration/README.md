# 大模型协作记录

这里收录 38 个需求或现场反馈窗口中的 77 条真实消息（38 条用户、39 条模型），围绕算法、HLS、时序、工具排错与机械联调整理了 26 项复盘。

| 文件 | 内容 |
| --- | --- |
| llm_collaboration_expanded.docx / .pdf | 完整协作复盘，供阅读和提交 |
| llm_collaboration_expanded.md | 同内容的可维护文本 |
| records/history_transcript.md / .jsonl | 77 条完整消息与时间、角色、来源行号、哈希 |
| records/history_index.json | 记录选取范围与计数 |
| llm_collaboration.md | C01—C06 六项补充复盘，保留原索引 |
| records/dialogue_excerpts.md / .jsonl | 早期 9 段摘录，可能与 77 条消息重叠 |
| reconstructed_examples.md | 8 套事后提炼的复用提示词 |
| skill_extraction.md | 从失败和观察到七项技能的提炼过程 |
| reproduction.md | 核查命令与预期结果 |

原文保留当时的错误和阶段方案，当前状态以设计报告及复盘结论为准。8 套模板是后来整理的工作流，不计入历史交互。时间为 UTC，北京时间需加 8 小时。

从包根执行 `python -B sim/skill_validation/check_evidence.py`，可核对 22 份工程证据、9 段摘录及 77 条消息的哈希。来源路径和行号用于追踪原记录。
