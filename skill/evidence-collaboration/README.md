# 07 大模型协作与纠错记录

## 适用场景

用于整理算法设计、HLS/RTL 排错和报告编写中的真实协作过程。输入包括原始目标、真实消息、工具报告、时间、版本与未解决项。

## 使用方法

按 `prompts/record.md` 限定输入证据，使用 `templates/record.json` 记录假设、提示词、回答、反证、修正和复测。没有对话时写工程复盘，不补造问答。在包根运行 `python -B sim/skill_validation/check_evidence.py` 核对现有记录。

## 提炼来源与已验证效果

已整理 77 条真实消息（38 用户、39 模型）、26 项复盘和 22 份工程证据。另保留 9 段可能重叠的旧摘录，8 套新提示词单独标注为事后提炼。D02/E19 的容量纠正、E06/E07 的收据矛盾和 D09 的实物反馈，让记录加入证据版本和纠错字段。原文在 report/collaboration/records/。

## 失效条件

缺失上下文不能从现有记录完整恢复。模型名只采用对应 turn_context，时间为 UTC。包内哈希确认内容与索引一致，对原会话的核查仍需查来源行号。

测试定义在 `../tests/test_skills.py`，统一收据在包根 `sim/skill_validation/`。E 编号见 `../evidence/index.json`，D 编号见包根 `report/collaboration/records/dialogue_excerpts.jsonl`。
