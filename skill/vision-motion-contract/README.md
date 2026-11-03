# 05 视觉结果与机构任务契约

## 适用场景

用于把视觉 JSON 接到抓取或定位调度器之前。上游需先完成像素到工作平面的标定，并提供 cm 坐标、generation、age_ms、ROI 有效性、类别与溢出状态。

## 使用方法

在包根预览候选任务：
```powershell
python -B skill/vision-motion-contract/scripts/preview_task.py --result skill/vision-motion-contract/examples/frame.json --config skill/vision-motion-contract/examples/config.json
```
配置定义原点、轴向、范围和类别路由。一次选一个任务，动作完成后重新取帧。脚本只输出预览。

## 提炼来源与已验证效果

9 项 Vision 测试和 1 项 CLI 预览通过，样例 [9.25,10.5] 转为 [1.25,−2.50]、仓2；13.001cm 的越界坐标不会因舍入而通过。E18 的坐标→0 OK→仓号→N OK 握手、E02 的 frame token、D09 的源码与实物差异，形成了有效性和版本一致性检查。

## 失效条件

不提供标定、避障、逆运动学和物理控制。0.01cm 是编码分辨率，不是定位精度。age_ms 要由上游可靠时钟产生，类别与仓位须重新配置。

测试定义在 `../tests/test_skills.py`，统一收据在包根 `sim/skill_validation/`。E 编号见 `../evidence/index.json`，D 编号见包根 `report/collaboration/records/dialogue_excerpts.jsonl`。
