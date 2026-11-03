# 03 Vivado 实现报告筛查

## 适用场景

用于离线复核英文 Vivado timing summary、check_timing、DRC 和 utilization。四份输入应来自同一次实现。

## 使用方法

在包根运行下面的负例，预期退出 1：
```powershell
python -B skill/vivado-report-gate/scripts/check_reports.py --timing skill/evidence/e12_timing.rpt --check-timing skill/evidence/e13_check_timing.rpt --drc skill/evidence/e14_drc.rpt --utilization skill/evidence/e15_utilization.rpt
```
正常项目替换四个输入，默认资源上限为 0.9，可按预算调整。

## 提炼来源与已验证效果

6 项 Report 测试通过。E12—E15 的修复分支负例解析出 WNS −0.247ns、WHS −1.111ns、LUT21857，并检出6个输入和9个输出缺少延时约束。D06 中局部路径改善但视频域退化，E11 中机械域通过但相机域失败，由此增加了全局检查。该 LUT 值属于负例分支，主线资源见 build/reports。

## 失效条件

不替代 CDC、时钟例外、电气或实物验收。默认把 DRC Error 作为硬失败，其他严重度保留计数供人工审查。未覆盖项仍需检查，输出 full_signoff 始终为 false。

测试定义在 `../tests/test_skills.py`，统一收据在包根 `sim/skill_validation/`。E 编号见 `../evidence/index.json`，D 编号见包根 `report/collaboration/records/dialogue_excerpts.jsonl`。
