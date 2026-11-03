# 构建工程

环境：PowerShell、Python 3.10+、Vivado / SDK / Vivado HLS 2019.1，以及 xc7z020 器件支持。将包解压到短路径，进入包根，把下面的工具路径改成自己的安装位置。构建期间不要同时打开另一个会修改相同工程的 Vivado 会话。

```powershell
$pkg = (Get-Location).Path
$run = Join-Path (Split-Path $pkg -Parent) 'rebuild_01'
$vivado = 'D:/XILINX/Vivado/2019.1/bin/vivado.bat'
$hls = 'D:/XILINX/Vivado/2019.1/bin/vivado_hls.bat'
$env:VISION_VIVADO_EXECUTABLE = $vivado
```

## 1 创建工程

```powershell
python -B build/run_tool.py --exe $vivado --output "$run/create_logs" --require-marker -- -mode batch -nojournal -log "$run/create_logs/vivado.log" -source "$pkg/build/create_project.tcl" -tclargs "$run/create" create
```

脚本复制 BD 到新目录，注册包内 IP，添加 RTL 和 XDC，执行 BD 验证与 IP 锁定检查。顶层为 `camera_claw_top`，器件为 `xc7z020clg400-2`。这一步不跑综合。

## 2 综合和实现

另取一个新目录，执行完整构建：

```powershell
python -B build/run_tool.py --exe $vivado --output "$run/pl_logs" --timeout 14400 --require-marker -- -mode batch -nojournal -log "$run/pl_logs/vivado.log" -source "$pkg/build/create_project.tcl" -tclargs "$run/pl" pl
```

输出有工程、bit、HDF、hwdef、DCP，以及 timing、utilization、check_timing、CDC、DRC 报告。检查报告后再继续发布；命令成功不代表时序通过。

`reports/integrated_route/` 是 11 月 2 日原控制整合版的历史路由报告，LUT 21208，FF 27530，BRAM tile 22，DSP 54，WNS −0.247 ns，WHS −1.111 ns。本包没有重新生成这些报告。

## 3 重建 SDK 软件与 BOOT

```powershell
$env:XILINX_SDK_ROOT = 'D:/XILINX/SDK/2019.1'
python -B build/build_software.py --hdf "$run/pl/camera_claw_top.hdf" --bit "$run/pl/camera_claw_top.bit" --output "$run/sdk"
python -B build/pack_boot.py --bit "$run/pl/camera_claw_top.bit" --hdf "$run/pl/camera_claw_top.hdf" --app "$run/sdk/live_overlay.elf" --fsbl "$run/sdk/fsbl_2019.elf" --bootgen "$env:XILINX_SDK_ROOT/bin/bootgen.bat" --output "$run/boot"
```

SDK 脚本创建新硬件平台和 BSP，编译包内 7 个 C 单元，保持 App UART1、FSBL UART0。脚本检查编译错误、UART 配置、HDF 内 bit 和 DDR 缓冲 ABI。BOOT 脚本只生成镜像。新构建产品应整套使用；`board/release/` 是单独冻结的历史配套版本。

## 4 HLS 独立验证

颜色核先运行完整 C 仿真，再用不同输出目录运行 RTL 子集：

```powershell
python -B build/run_tool.py --exe $hls --output "$run/color_host" --timeout 14400 --require-marker -- -f "$pkg/build/hls_build.tcl" -tclargs object_color
$env:COLOR_RTL_TEST = '1'
python -B build/run_tool.py --exe $hls --output "$run/color_rtl" --timeout 14400 --require-marker -- -f "$pkg/build/hls_build.tcl" -tclargs object_color
Remove-Item Env:COLOR_RTL_TEST
python -B build/run_tool.py --exe $hls --output "$run/feature_hls" --timeout 14400 --require-marker -- -f "$pkg/build/hls_build.tcl" -tclargs feature
```

颜色核有 15 个文件案例和 1 个无效尺寸事务。RTL 子集是 6 个事务。feature 入口会复制 `data/feature_ppm/` 作为测试输入；当前 feature C 生成的是新候选，主工程使用冻结 IP RTL。

上述构建入口已完成离线语法、CLI、日志返回码和配套检查。迁移后的全量 Vivado、SDK、HLS 构建仍需在安装好 2019.1 的环境中执行；本次整理没有启动这些构建。
