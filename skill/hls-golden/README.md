# 02 HLS 黄金参考与报告核查

## 适用场景

用于 C/C++ 迁移到 HLS 后的数值和协议比对。需要独立 golden JSON、候选 actual JSON，以及同版本 csynth XML、cosim 报告和可选旧收据。

## 使用方法

在包根运行：
```powershell
python -B skill/hls-golden/scripts/compare_json.py --golden skill/hls-golden/examples/golden.json --actual skill/hls-golden/examples/actual.json
python -B skill/hls-golden/scripts/inspect_hls.py --synthesis skill/evidence/e03_color_csynth.xml --cosim skill/evidence/e04_color_cosim.rpt
```
整数默认精确比较；浮点只有在明确误差预算后才设置 `--atol`、`--rtol`。诊断旧收据时增加 `--receipt`。

## 提炼来源与已验证效果

Golden11、HLS5 和 1 项 CLI 比对通过。E06 把 RTL 写为失败，E07 的真实 Verilog 表为 Pass，E09 记录旧 runner 漏读报告文件；脚本现在读取真实表，并对冲突返回 1。颜色核解析得到 LUT2678、FF1906、DSP2、BRAM18=2、估计8.321ns。

## 失效条件

脚本不能证明 oracle 独立或样本充分。报告格式变化需要适配；数值一致不等于现场准确率，HLS 周期也不等于整机 FPS。

测试定义在 `../tests/test_skills.py`，统一收据在包根 `sim/skill_validation/`。E 编号见 `../evidence/index.json`，D 编号见包根 `report/collaboration/records/dialogue_excerpts.jsonl`。
