# 06 通用 PYNQ Overlay 迁移

## 适用场景

用于 PYNQ 更换算法、IP、Overlay 或板卡。输入是目标板生成的同名 bit/hwh、required_ips 契约、板卡和 PYNQ 版本；不绑定本题颜色、机构、地址或 IP 名。

## 使用方法

先运行离线样例：
```powershell
python -B skill/pynq-overlay-portability/scripts/preflight.py --bit skill/pynq-overlay-portability/examples/demo.bit --hwh skill/pynq-overlay-portability/examples/demo.hwh --contract skill/pynq-overlay-portability/examples/contract.json
```
demo.bit 是明文夹具。真实部署按 `references/pynq_runtime.md` 记录版本、核对来源、加载 Overlay、检查 ip_dict/register_map，再执行小规模寄存器或 DMA 黄金输入。

## 提炼来源与已验证效果

7 项 PYNQ 测试通过，覆盖缺 IP、寄存器错位、哈希错配、文件名不配套和坏 XML；将 accel_0 改为 filter_2 并更新契约后仍通过。方法从 E20 的硬件与软件同步、E02/E11 的分层验收中提炼，已验证范围是合成 HWH 离线测试。

## 失效条件

换板通常要重新生成 bit/hwh。同名不能证明成对构建；脚本不解析 bit 器件头，不验证完整 HWH schema、DMA 宽度/TLAST 或缓存一致性。目前没有真实 PYNQ 板测结果。

测试定义在 `../tests/test_skills.py`，统一收据在包根 `sim/skill_validation/`。E 编号见 `../evidence/index.json`，D 编号见包根 `report/collaboration/records/dialogue_excerpts.jsonl`。
