# 04 发布产物与依赖核对

## 适用场景

用于有上下游构建关系的工程归档，检查 bit、硬件导出、软件与启动镜像是否错配。

## 使用方法

按 `examples/manifest.json` 填相对路径、SHA256 和 depends_on，依赖哈希在真实构建时记录。然后运行：
```powershell
python -B skill/release-provenance/scripts/verify_manifest.py --root skill/release-provenance/examples --manifest skill/release-provenance/examples/manifest.json
```
示例用文本夹具演示。2019.1 工程顺序是 bit→HDF→SDK/BSP/ELF→BOOT，PYNQ 则使用 bit/hwh。

## 提炼来源与已验证效果

8 项 Manifest 测试通过，覆盖文件改变、缺失、父哈希错配、缺角色、循环依赖和目录外路径。E20 记录过 XSCT 在编译出错后仍出现成功标记的问题，因此核查需要结合日志和 ELF 新鲜度；E11 则提供配套构建与分阶段板测的经验。

## 失效条件

哈希只能确认文件和声明依赖自洽，不能证明依赖实际参与编译。仍需保存工具版本、命令、日志和源码版本；校验通过不代表时序或板测通过。

测试定义在 `../tests/test_skills.py`，统一收据在包根 `sim/skill_validation/`。E 编号见 `../evidence/index.json`，D 编号见包根 `report/collaboration/records/dialogue_excerpts.jsonl`。
