# 抓取停在坐标上空：本轮修复

用户现象为偶发到达目标坐标上空后不下降，尤其第三轮抓取或发送串口命令时。

## 可复现问题与改动

- 旧代码在到位后读取0x36位置时，只要一次回包丢失便中止整轮；复现日志为`baseline_sim_02/tb_stall_recovery_1.log`。这能解释停在上空的表现，但尚未证明实物每次异常都由它引起。
- `emm_link.v`对幂等读取超时/坏尾字节最多重试2次；请求发完才开始响应计时，清理残帧、等待空闲且有噪声超时。无有效位置不会下降；FD运动、FE停止、回零、保存零点均不重发。
- 新增`pc_report_mux.v`，允许动作中只读STATUS/PROGRESS，缓冲前台完成回包，避免查询应答推进抓放状态或覆盖动作应答。
- `PROGRESS`保留故障发生时状态/错误码；停止失败覆盖原错误文字时仍可定位原始位置读超时。协议见`D:/312312/FPGA/1/1/ceshi/AUTOMATIC_CONTROL.md`。

## 源码范围

独立源码：`D:/312312/FPGA/1/1/ceshi/rtl`。本轮修改`emm_link.v`、`claw_controller.v`、`command_parser.v`，新增`pc_report_mux.v`。

整合源码：`D:/312312/FPGA/toolchain/workspaces/camera_claw_candidate_20261102/control/rtl`。上述4个文件已经同步并注册到`mipi_hdmi_claw_preserve_repaired.xpr`；整合工程继续使用100 MHz优化的mapper/PWM，未以独立50 MHz版本覆盖它们。原画框软件输入保持冻结。

夹爪0°闭合、35°张开。正常协议仍为坐标→等待`0 OK`→盒号→等待`N OK`。

修改前独立RTL、测试台及协议分别保存在`original_rtl/`、`original_sim/`、`AUTOMATIC_CONTROL.md`；整合工程修改前文件在`integrated_before/`，哈希记录为`integration_changes.json`。`control_changes.patch`记录控制RTL差异。

## 验证证据

- `final_sim_01/result.json`：独立RTL原完整回归与5种故障注入通过，另有50/100 MHz、真实115200波特率链路专项。
- `progress_sim_02/result.json`：新增PROGRESS运动期间查询与停止失败后的原始故障保留通过。
- `integrated_sim_01/result.json`：整合版16项全量回归全部通过，共805条断言；已核对当前整合RTL与测试输入快照逐文件哈希相同。
- `progress_sim_01`保留早期测试台失败证据；输出数值正确，但Vivado 2019.1的`$sscanf`需显式匹配`+`符号。修正测试台后通过，未为此修改RTL。

长期测试入口`1/1/ceshi/scripts/test.ps1`调用`toolchain/claw_regression.py`，冻结输入并检查PASS计数和Fatal/Error。测试包括先成功两轮再在第三轮注入故障、所有24个盒内点与计数回绕、STOP、回零及启动、查询与完成回包碰撞、链路噪声与取消。

## 首轮构建状态（固化前历史记录）

整合综合和布局布线已完成，45576条可布线网络全部完成，布线错误0；DRC报告无Error项。原画框软件19份输入文件与冻结清单一致，见`source_checks.json`。当前没有生成或下载本轮修复bit，没有写SD/Flash，也没有执行实物抓放。以前已通过的板测属于旧bit，不能作为本轮验证。

机械控制所在`clk_fpga_0`为100 MHz，同域最差setup余量+0.244 ns、hold余量+0.061 ns，通过。全局setup −0.247 ns、hold −1.111 ns，两个失败端点均在`camera_hs`相机输入域，与原整合基线数值相同。寄存器缺时钟0、未约束内部端点0；外部I/O仍有6个输入/9个输出缺少延时约束提示。CDC保留7条相机Critical项（CDC-7×2、CDC-10×3、CDC-11×2）。完整报告在`routed/`；控制时钟路径报告为`routed/claw_clock_timing.rpt`。

本轮不修改相机约束来掩盖报告。`toolchain/README.md`发布流程明确要求“negative setup/hold slack and DRC errors block this task”，因此时序未通过，不生成正式bit/BOOT，也不下载新修复版。新鲜`routed/camera_claw_top.hdf`仅用于编译核对：HDMI应用/BSP/FSBL已全部编译成功，App UART1、FSBL UART0及显示缓冲符号/尺寸检查通过，见`software/build_status.json`；无bit匹配，不用于部署。修复代码与仿真已完成，实物异常根因及修复效果仍需新程序上板后验证。

## 后续固化进展（2026-11-04）

用户随后在已知相机时序限制的情况下，明确要求下载最新代码并保持掉电不丢失。现场镜像已完成新 bit → 匹配 HDF → 更新 SDK 硬件 → 重建 BSP/FSBL/App → BOOT 打包，位于 `toolchain/jobs/20261103_claw_stall_persistent_01/`。本目录首轮状态已保留为 `delivery_status_before_persistent_deployment.json`；实时结果见 `delivery_status.json` 与镜像目录 `manifest.json`。

已备份并验证原 Flash 的低 4 MiB。标准 `program_flash` 未能识别 Flash，改用 SDK 2019.1 `XQspiPs` 驱动，完成受限范围擦写、空白检查、板端逐字节比较和电脑端完整读回比较。实际 Flash 读回的 BOOT SHA256 为 `1a2fe1a6a6c7ed55dcacd9177838bd8d715073e6a35209d3a96923c8cc20835a`，与目标镜像完全一致。固化证据位于镜像目录的 `sdk_commit_20261104_001912_490339/`。

固化成功不代表启动或实物抓放已经验证。启动结果以镜像目录后续 `reset_*/status.json`、`observe_*/status.json` 为准；本轮只发送 STATUS/PROGRESS 查询。相机时序限制仍在，当前镜像不标为正式时序签核版。

用户随后实际断电上电并确认画面和物体框正常。镜像目录 `cold_boot_validation.json` 记录 QSPI 模式为 1、运行应用与冻结 ELF 匹配、相机帧 2363→2386、5 个物体且错误计数为 0，冷启动与画框验证通过。`deploy.py` 已修正观察阶段判定：2 表示连续运行，3 表示暂停快照；原始误判记录保留，程序镜像不变。

串口接收链路仍待解决：COM12 能打开，重插后 Python 和 .NET 均未收到 STATUS 字节；被动采样看到 T15 输入及 T14 回传活动，BREAK 测试确认适配器 TXD 到达 T15。当前等待适配器自身 TXD/RXD 回环检查，未发送运动命令，不能将本修复版的多轮实物抓放标为通过。
