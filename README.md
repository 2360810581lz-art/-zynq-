# 基于 Zynq 的视觉引导分拣装置

这个项目用相机识别桌面物体的位置和外观，通过 Zynq 上的视觉处理与 PL 控制器，组织抓取、转运和分类投放。采集和显示走 MIPI、VDMA、DDR、HDMI 链路；PS 负责当前视觉算法与配置，PL 负责视频接口和机构执行。我们另外实现了 HLS 颜色识别候选核，用来验证重复像素计算迁移到 PL 的方案。

本包对应 2026 年 11 月 4 日恢复的原控制整合版本。板卡是 AX7Z020B，器件为 `xc7z020clg400-2`，使用 Vivado / SDK / Vivado HLS 2019.1。PS 运行 standalone，App 用 UART1，FSBL 用 UART0。

## 先看什么

- [设计报告](report/design_report.pdf)：总体方案、算法、接口、资源和验证结果；同目录有可编辑 Word 稿。
- [技能包](skill/README.md)：七项技能，包含通用 PYNQ Overlay 检查方法。
- [大模型协作记录](report/collaboration/README.md)：77 条真实消息、26 项复盘和 8 套复用提示词。
- [构建说明](build/README.md)：从新建 Vivado 工程到生成 BOOT 的步骤。
- [提交条款对照](report/submission_checklist.md)：各项材料的位置和当前状态。

## 目录


| 目录    | 内容                                         |
| ------- | -------------------------------------------- |
| src/    | 当前整合 RTL、BD、IP、PS 源码；两个 HLS 工程 |
| sim/    | HLS 结果、机械差分仿真输入与轨迹、离线检查   |
| build/  | PL、SDK、HLS、BOOT 构建入口和历史报告        |
| board/  | 配套 bit、HDF、ELF、BOOT，板测记录和板卡资料 |
| data/   | HLS 测试图及四张历史场景素材                 |
| skill/  | 提示词、模板、校验脚本、测试和提炼依据       |
| report/ | 设计报告、协作原文、技能提炼和来源映射       |

## 先做离线检查

安装 Python 3.10 以上，解压到短路径，例如 `D:/sort/`，进入 `autonomous_sorting` 后运行：

```powershell
python -B sim/verify_package.py --output ../package_check.json
python -B skill/run_checks.py --output ../skill_checks.json
python -B sim/skill_validation/replay.py
python -B sim/skill_validation/check_evidence.py
python -B sim/calibration/check_traces.py --output ../trace_check.json
```

这些命令使用 Python 标准库，不需要联网或连接板卡。预期结果为完整性检查通过、48 项技能测试通过、7 组回放符合预期，以及三版本机械轨迹一致。回放中的旧收据冲突和负时序是两个预期拒绝案例。回放会更新运行收据，原包哈希应在运行回放前检查。

硬件重建按 [build/README.md](build/README.md) 执行，生成物写到包外的新目录。修改 PL 后依次生成新 bit、新 HDF，再重建 SDK/BSP/ELF 和 BOOT。


`manifest.json` 记录逐文件哈希，`board/release/release.json` 记录配套镜像，`report/provenance/source_map.json` 记录源码和证据的来源。第三方设计及资料的说明见 [third_party_notices.md](report/third_party_notices.md)。

## GitHub 发布

仓库保留完整交付材料。首次上传、换行设置和许可范围见 [GitHub 发布说明](report/github_publishing.md)。
