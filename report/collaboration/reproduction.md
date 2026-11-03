# 复测协作材料和技能

安装 Python 3.10+，进入包根 `autonomous_sorting`。以下命令不需要板卡或第三方 Python 包：

```powershell
python -B sim/verify_package.py --output ../package_check.json
python -B skill/run_checks.py --output ../skill_checks.json
python -B sim/skill_validation/replay.py
python -B sim/skill_validation/check_evidence.py
```

预期依次为文件完整性通过、48 项测试通过、7 组回放符合预期、22 份证据/9 段摘录/77 条消息哈希通过。回放会更新收据，先核原包哈希。

`sim/skill_validation/results/` 的两个 passed=false 是预期结果：historical_receipt_conflict 检出旧收据和 RTL 表冲突，current_timing_rejected 检出修复分支负时序及缺失约束。其余 5 项为正常样例。PYNQ 和 manifest 使用合成夹具；demo.bit 不能用于下载。

复盘正文有 26 项案例，提示词模板另有 8 套。9 段旧摘录可能与 77 条消息重叠，数量不相加。换题目时保留示例自检，再换输入并增加一个错误样例验证拒绝分支。

完整工程重建按包根 `build/README.md` 执行。主线为 Vivado/SDK/HLS 2019.1、xc7z020、standalone；通用 PYNQ 技能另有独立运行指南，不影响主工程构建。
