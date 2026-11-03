# FPGA 与 PYNQ 技能包

这七项技能来自本项目在软硬件划分、HLS 验证、时序排查和机构联调中遇到的问题。我们把可重复的判断整理成提示词、模板和脚本，换题目时替换参数与输入即可使用。

各目录的 README 写给使用者，SKILL.md 供支持文件系统技能的大模型客户端读取。脚本使用 Python 3.10+ 标准库，离线运行；不发送串口命令，不下载硬件。

## 先运行示例

在包根执行：

```powershell
python -B skill/run_checks.py --output ../skill_checks.json
python -B sim/skill_validation/replay.py
```

第一条运行 48 项测试，第二条回放 7 组输入。旧收据冲突、负时序两组预期被拒绝。测试收据在 `sim/skill_validation/`，定义在 `tests/test_skills.py`。

## 选用哪一项

| 目录 | 用途 | 已验证效果 |
| --- | --- | --- |
| ps-pl-partition | 算子、存储与接口预算，决定 PS/PL 划分 | 真实资源与容量复算，纠正单特征图容量判断 |
| hls-golden | 黄金参考比对与 HLS 报告读取 | Golden11、HLS5、CLI 比对1；检出旧收据冲突 |
| vivado-report-gate | 时序、约束覆盖、DRC 与资源筛查 | Report6；真实负时序报告被拒绝 |
| release-provenance | 配套产物哈希与依赖核对 | Manifest8，覆盖错配、缺文件和循环依赖 |
| vision-motion-contract | 视觉结果到机构任务的离线预览 | Vision9、CLI 预览1，覆盖混帧、超界与过期 |
| pynq-overlay-portability | 通用 Overlay 契约和换板调试流程 | PYNQ7；离线 HWH 元数据与更名测试 |
| evidence-collaboration | 提示词、模型回答、证据与纠错记录 | 77 条消息、9 段旧摘录、22 份证据哈希核对 |

每项 README 都列出适用场景、命令、观察来源、效果和失效条件。`pitfalls.md` 按故障现象列排查步骤；`evidence/index.json` 索引 22 份冻结工程证据。E 编号指这些证据，D 编号指 `report/collaboration/records/dialogue_excerpts.jsonl`。

## 换题目使用

可以复制整个 `skill/` 保留全部测试，也可以取用某一项并换成自己的输入。先跑附带示例，再构造一个错误输入确认会被拒绝。器件资源、报告格式、坐标标定、IP 名与寄存器偏移都需要按新工程填写。

通用 PYNQ 技能不绑定本题算法和板卡。`examples/demo.bit` 是明文测试夹具，真实部署要用目标板重新构建的 bit/hwh。现有测试只覆盖离线检查，尚无真实 PYNQ 板测记录。

测试在 Windows、Python 3.12.14 下通过。尚未在 Linux 实跑。提示词需要工程人员复核输出，脚本结果也只覆盖输入字段和测试范围。
