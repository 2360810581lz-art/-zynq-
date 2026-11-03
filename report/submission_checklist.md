# 比赛提交内容对照

| 要求 | 材料位置 | 状态 |
| --- | --- | --- |
| 3.3.5.1 完整源码 | src/rtl、bd、ip、ps | 当前原控制整合版依赖 |
| HLS 源码 | src/hls/object_color、feature | 颜色核完整；feature 为参考快照，主工程使用冻结 RTL |
| 仿真与验证 | sim/hls_*、calibration、skill_validation | 历史结果及本次离线复测 |
| 综合实现与指标 | build/reports、sim/hls_* | 真实历史报告；整合版时序仍有负余量 |
| 上板与实测比对 | board/release、board/evidence | 配套产品、指定坐标抓放、图像逐像素比对和恢复记录 |
| 可复现脚本 | build/README.md、sim/calibration/README.md | PL→HDF→SDK→BOOT、HLS、XSim；迁移入口未重新全量原生构建 |
| 3.3.5.2 技能包 | skill/README.md 及七项子目录 | 各项含场景、方法、来源、效果和失效条件 |
| 通用 PYNQ Skill | skill/pynq-overlay-portability | 跨题目元数据契约；7 项离线测试，待真实板测 |
| 3.3.5.3 设计报告 | report/design_report.docx / .pdf / .md | 背景、框图、划分、接口、优化、技能提炼与复现 |
| 大模型协作记录 | report/collaboration/ | 77 条真实消息、26 案例、8 套提炼提示词，另保留9段旧摘录 |
| 3.3.5.4 目录与命名 | 根目录、manifest.json | 按推荐目录组织；英文路径，README/SKILL 使用约定名称 |

剩余验收包括相机全局时序和 CDC、视觉直接调度机构、六仓映射和全范围标定，以及恢复后冷启动和抓放回归。扩展板 PCB 和机械 CAD 源文件未收集到。详细状态见设计报告。
