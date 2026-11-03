# Dynamic live foreground boxes

当前离线状态与证据以 [PL迁移](PL_MIGRATION.md) 和 [续接卡](../../../PROJECT_RESUME.md) 为准。
板卡断电，集中完成候选后统一板测。九个独立IP审阅212004通过，四历史图的实际七核C串接211918
与生产逐像素及最终框一致；根/游程物化已使用实际root replay算法核，DDR调度仍为host。
绿色邻域统计204346通过20主机/8RTL；共享统计211417通过36主机/12RTL，mode1替代独立原黄保护/绿种子核，实际绿色四核C串接211840逐像素核对四历史图通过。
共享后省1771 LUT，九核保守预算50169/53200 LUT，剩3031；mode1增加一遍像素重放，资源节省不代表提速。以上是未接入发布镜像的 PL 候选背景，不能据独立 IP 宣称整机提速。
当前 PS 合并版已经提供颜色/形状标签，未知类别仍以有效位清零表示。SD及全部回退不动。此前状态快照保存于
`toolchain/jobs/20261017_201933_context_maintenance/`。

这是PS实时检测和CPU画框的唯一生产维护入口。1280×720 BGR输入，640×360检测，实际黑边决定ROI，
整数颜色/opening/support/8连通域/侧面合并，每帧重算数量，支持0～8，超限报告overflow。
当前合并版沿用 Python 参考程序的颜色/形状分类顺序，并把标签写入原 `match_error` 字段；HDMI
仍显示黄色区域框、绿色物体框和每个物体的颜色/形状两位十六进制标签。标签用于下一步抓取路由，
尚未连接机械臂 UART，也没有把六盒坐标硬编码进检测器。

## Python 识别与板端画框的合并

参考实现为 `camera_base_2019_1_fixed/tools/vision_reference/vision_reference.py`，板端实现为
本目录的 `detector.c/.h`。板端仍在 640×360 半分辨率上完成实时分割和画框，同时为每个框输出
颜色 ID、形状 ID、有效位和保守置信度；原有 `LiveResult` ABI 保持 268 字节，因此不需要修改
现有显示缓冲协议。

颜色 ID：`1 red`、`2 orange`、`3 yellow`、`4 green`、`5 blue`；形状 ID：`1 cylinder`、
`2 triangular_prism`、`3 pentagonal_prism`、`4 cube`。`class_id = (color_id << 4) | shape_id`，
例如红色圆柱为 `0x11`、蓝色立方体为 `0x54`。`match_error` 的位布局为：bits 0--3 颜色、
bits 4--7 形状、bit 8 颜色和形状均有效、bits 16--23 为 Q0.8 置信度。未知或未通过门禁的
标签会清除有效位，不能触发抓取。

板端 `LiveDraw` 将颜色和形状 ID 画在框左上角；电脑端不需要 HDMI 采集卡，识别结果直接随
OV5640 的 HDMI 输出到独立显示器。Python 版本仍可用于离线验证和生成带文字的 `overlay.png`。

## 当前板卡程序身份

2026-11-01 的只读 JTAG 代码比对显示，板卡当时运行的是实验程序
`toolchain/jobs/20261017_154422_mipi_trial_boot/vision_trial.elf`，SHA256
`b7cfe92cb0cdbf1a3d4bd128f100d607f94ed6ad0d0089a3cd8f8dbcbb84d512`。它不是本次合并版，
也不是最终机械抓取发布版。新合并 ELF 位于
`toolchain/jobs/20261101_171327_live_overlay/live_overlay.elf`，SHA256
`f7f9123271f8753cd6123e3a3865a8f7e5f0da625903443aba1e7a910db5cd05`，当前尚未下载到板卡，
SD 卡和 BOOT.bin 均未修改。

随后一次只读状态读取因 XSCT 报告 `no targets found with "name =~ APU"` 而未能取得运行中的
`vision_status/raw/shown`；该操作没有写寄存器、下载 ELF、复位板卡或发送机械动作。板卡重新
上电并且 JTAG target 可见后，再执行只读读取，确认新程序部署前后的实际框数量和坐标。
状态见 [续接卡](../../../PROJECT_RESUME.md)，PL候选见 [DMA维护页](dma_two_phase/README.md)。

2026-10-15 的“板卡断电、停止视觉验收”是历史维护记录；本轮仅生成并验证新的 C/ELF，未改 SDK、PL 或 SD，也没有把新 ELF 加载到板卡。
独立PS/display模型、完整禁用ARM构建和相机审计通过，整机相机输入时序仍阻塞发布。

## 当前 CPU 版本和板级证据

[180257普通构建](../../jobs/20261013_180257_live_overlay/build_status.json)包含黄色边界修整，
ELF SHA `ede37c72b389991dab8d04193a57286d2cb7c0a30471db385fe681250ab2fa8e`。
[181016BOOT](../../jobs/20261013_181016_live_boot/manifest.json)
SHA `2dc86e654176a8976c647c86150f26564a679f90894b4edb9887c9fdbc1ad85c`已写SD。
沿用相机PL/BSP；bit SHA `f4139c6d4fc827f59cd04789f877f19916e8fb2948ac218e8d61c5ad5c4be100`。

| 证据 | 通过范围 |
| --- | --- |
| [构建/回归汇总](../../jobs/20261013_1759_yellow_bounds/maintenance/release_evidence.json) | 21历史＋17实景、7黄色几何、161稳框、20反光ROI守卫及ARM严格编译 |
| [0～8数量覆盖](../../jobs/20261014_224753_live_probe/count_coverage.json) | 同ELF、九场操作后短窗口、576轨迹/九张显示回放，用户对应数量实屏确认 |
| [SD写入/回读](../../jobs/20261014_220128_livedraw_sd_deployment/sd_deployment.json) | 新BOOT SHA一致，旧BOOT备份 |
| [一次冷启动](../../jobs/20261014_220746_live_probe/startup_acceptance.json) / [一次复位](../../jobs/20261014_221107_live_probe/startup_acceptance.json) | 自动完整实时画面、区域框/六框，用户确认 |
| [短遮挡后恢复](../../jobs/20261014_225709_live_probe/acceptance.json) | 操作后四框/64帧/显示一致，恢复延迟未测 |
| [绿色移位端点](../../jobs/20261014_230236_live_probe/movement_comparison.json) | 人工对应绿框跟随，无旧位置残框，非运行时ID |
| [最新四框短测](../../jobs/20261014_231050_live_probe/acceptance.json) | 用户报告旋转后正常；8状态/64帧四框/显示零差异，独立旋转覆盖未确认 |

各数量场核对raw/ROI/shown/BoxState和921600显示像素，DMA错误0。
0～4短场约10FPS、5～8约7.5FPS，是CPU实测，不是PL成绩。
静态短窗口不覆盖拿放/遮挡/移动全过程、响应延迟或通用姿态/光照。
七场黄色框左边曾单帧变化14px，只有末帧原图，原因未确定，未放宽稳框。
区域框偶发偏差按用户要求暂缓，不宣布角点定位完善。

[214119反光板测失败](../../jobs/20261014_214119_live_board/board_status.json)、
[221503八场初始失败](../../jobs/20261014_221503_live_probe/README.md)及所有原收据保留。
八场rc=-2为区域定位失败，不是取帧等待超时；随后有限probe恢复，不证明外部复位原因或永久卡死。
[旧维护页](../../jobs/20261014_234923_camera_firmware_audit/maintenance/before/live_overlay_README.md)保存详细轨迹/历史，其相对链接按本维护目录解释。

## 检测和诊断契约

色相中心7/179/13/23/60/110；red/orange共用warm，S门限warm82/yellow50/green-blue20，V≥40。
warm在hue14～20、r−g≤24、hi≥80时按yellow/S≥50重判。
面积≥500半分辨率像素；蓝色有强核心/亮度分支守卫，绿色弱支持用3×3局部密度。
小侧面按面积比例/投影/实际像素距离≤2半像素保守合并，独立贴近物体未充分覆盖。
raw每帧重算，shown仅限定稳框，不保留消失物体；不能直接删所有权/取帧等待提速。

黄色仅修整已接受组件：5×5完整支持主体向外补4半像素，限制原框内，保留≥95%组件像素，否则原框。
没有改面积/数量门禁、颜色掩码、合并、稳框或HLS前端。
[最终对照](../../jobs/20261013_1759_yellow_bounds/compare_final/verification.json)涵盖细分支、矩形、尖角/细长侧面，异常底625→615。
两张新增实景及历史回归通过，仅排除用户指定两张黑框失败历史图。

ROI缓存依赖实际黑边，短遮挡最多复用1200ms，过期/黑场清锁，旧generation拒绝。
反光候选需暗带支持/对边长度门禁，失败安全拒绝错误外扩ROI。
CPU ABI：36词live_status、268字节结果、72字节ROI、280字节稳框、64×80词trace。
profile独立1040字节默认关闭，edge_capture/profile禁止打包生产BOOT。

probe_board.py先核ELF可执行段、seqlock采样；--frames 0不暂停/采图。
暂停回放绑定新pause token、输入前ROI/generation/时间，比较raw/shown/BoxState/ROI和全部显示像素。
inspect_frame.py从已暂停冻结图导出掩码/门禁，review_box_trace.py核末条trace与现场。
独立PL的vision_status[32]不是此ABI，不能用基线pause/probe协议操作。

## PL接入和发布边界

架构reduce → 同帧PS ROI → color/fork/ROI/opening，PS沿用当前detector/support/CCL/稳框/HDMI。
[黄色HLS C→PS](../../jobs/20261013_182700_yellow_pl_equivalence/verification.json)两张实景字节/结果/显示等价。
[PS/display234157](../../jobs/20261014_234157_vision_dma_ps_handoff/verification.json)27＋3＋5＋6组/19帧合成增减通过。
[完整ARM234342](../../jobs/20261014_234342_vision_dma_ps_board_compile/verification.json)10个C源严格编译/链接及布局检查；
release flag=0，旧BSP仅编译兼容，**不是上板版本**。
[相机审计234923](../../jobs/20261014_234923_camera_firmware_audit/verification.json)确认720p表字节/实际调用一致。
新软件模型没有新RTL/板测，不能据此声称加速FPS。

基线没有协调器，禁止访问0x44000000/0x44100000。
[整机refine](../../jobs/20261012_230933_vision_target_refine/verification.json)video+0.013/+0.056ns，整机−0.247/−1.111ns，
相机setup/hold各两个失败端点。
[冻结DCP复核181030](../../jobs/20261015_181030_335346_camera_timing_diagnosis/status.json)重现同值，
当前280MHz假设（周期3.571ns）；旧−0.843ns来自另一构建的140MHz假设，不能混用。
诊断入口绑定DCP自带时钟和同目录冻结XDC，六项短回归通过，无新实现/板测。
缺实际HS周期/占空比、双lane双边沿input-delay、参考点和偏斜/抖动依据；不猜tap或放宽约束。
[时序审计/参数要求](../../jobs/20261015_181030_335346_camera_timing_diagnosis/maintenance/TIMING_MODEL_REVIEW.md)保存来源和后续顺序。
补依据/整机门禁后，从通过目标fresh bit→HDF→SDK hardware/BSP→匹配ELF/BOOT→必要板测。
仅上电不能证明时序收敛。8.064ms/7,372,800bytes每帧只是像素下限/有效负载。

## 常用入口和维护

以下为CPU流程，当前断电不执行board/probe：

```powershell
.\toolchain\.venv\Scripts\python.exe -X utf8 -B toolchain/vision_workbench/live_overlay/run.py build
.\toolchain\.venv\Scripts\python.exe -X utf8 -B toolchain/vision_workbench/live_overlay/run.py board --job <build-job> --expected-count <actual-count> --skip-camera-restart
.\toolchain\.venv\Scripts\python.exe -X utf8 -B toolchain/vision_workbench/live_overlay/probe_board.py --job <build-job> --samples 100 --frames 0 --vdma-profile
.\toolchain\.venv\Scripts\python.exe -X utf8 -B toolchain/vision_workbench/live_overlay/run.py boot --job <build-job>
```

expected-count只是验收参数，不进检测。stock fpga.ps1 boot打包旧SDK app；LiveDraw用本入口。
旧20261010_183252 SD BOOT备份在20261014_220128_livedraw_sd_deployment/BOOT_before.bin；普通034537候选可回退。
三份旧维护页/未引用RTL临时备份归档至234923/maintenance/before，维护页压缩更新。
保留原图/冻结inputs/失败收据/旧BOOT/原工程备份/final_20260929。
Vivado/SDK/HLS2019.1、app UART1/FSBL UART0、原生任务串行共享锁。
新依赖/缓存/输出留D盘toolchain；不重复无关长仿真，需实际操作停止。
