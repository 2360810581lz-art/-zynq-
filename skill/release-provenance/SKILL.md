---
name: release-provenance
description: Verify local artifact hashes and declared build ancestry for hardware software releases without programming a board.
---

# 发布产物依赖与哈希核对

按真实构建顺序填写manifest，冻结产物后运行 scripts/verify_manifest.py。依赖不一致时重建下游并生成新清单，保留旧清单供审计。报告仅写完整性和声明依赖的核对结果。

使用场景、命令、验证记录与失效条件见 [README.md](README.md)。仅将其中“本项目实例”视作实例，其他题目应替换参数和输入。运行本技能不会扩大对实际设备操作的授权。
