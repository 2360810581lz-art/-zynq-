# 机械控制差分验证

这里保留恢复前做过的三版本差分试验。`original_calibrated` 是 50 MHz 原标定版，`known_good_integrated` 是曾抓放成功的 100 MHz 整合版，`current_integrated` 是当时的卡顿修复候选。最后这个名称沿用历史命名，不是当前发布源码；当前原控制恢复版在 `src/rtl/control/`。

每组包含 RTL、测试台、原始仿真日志和解码轨迹。历史结果各通过 354 条断言，产生 183 个电机报文和 5760 个舵机目标事件。忽略事件时间后，三组轨迹一致。

在包根重算现有记录：

```powershell
python -B sim/calibration/check_traces.py --output ../trace_check.json
```

重新运行 XSim 时指定本机 Vivado 路径和新的包外输出目录：

```powershell
python -B sim/calibration/run_xsim.py --bin 'D:/XILINX/Vivado/2019.1/bin' --output ../motion_sim_01
```

脚本只启动仿真，复制冻结输入后依次编译、展开和运行，再比较轨迹。仿真采用加速时钟和等待参数，检查运动命令与舵机目标，不代表真实 PWM 周期、机械精度或物理回零。

原标定参数保持不变：夹爪闭合/张开 0°/35°，底座启动 45°，中心伸缩 1100 脉冲/底座 135°，抬升 −1800、抓取下降 1200、投放下降 2500 脉冲，速度 60 RPM、加速度 100。源码锁在 `report/provenance/restore_manifest.json`。READY 定时标志不能用作物理回零确认。
