# 上板与历史验证

`release/` 保存 11 月 4 日恢复后冻结的配套文件：`boot.bin`、`camera_claw_top.bit`、`camera_claw_top.hdf`、`fsbl_2019.elf`、`live_overlay.elf`。哈希与验证状态见 `release/release.json`。App 使用 UART1，FSBL 使用 UART0。

## 使用冻结镜像

先在包根执行 `python -B sim/verify_package.py`。SD 启动时，将 `release/boot.bin` 复制到 FAT 分区并命名为 `BOOT.BIN`，按 AX7Z020B 手册设置启动方式。镜像有控制输出，通电前应核对电机和舵机供电、接线与机构位置。

需要 JTAG 调试时，先生成脚本：

```powershell
python -B board/prepare_jtag.py --output ../jtag_01
```

此命令只解出匹配的 `ps7_init.tcl` 并生成 `run_jtag.tcl`，不连接板卡。连接目标板、在 XSCT 中确认目标后，运行生成的脚本：

```powershell
& 'D:/XILINX/SDK/2019.1/bin/xsct.bat' '../jtag_01/run_jtag.tcl'
```

该脚本复位系统、加载 bit、初始化 PS 并下载 App。新构建版本应按 `build/README.md` 更新全部配套文件。

## 结果在哪里

| 目录 | 验证内容 |
| --- | --- |
| evidence/historical_ps/ | 10 月 14 日的 4 目标与 6 目标视觉测试、逐像素比对 |
| evidence/integrated_20261103/ | 两次 `(0,0)` 到 1 号盒抓放、显示缓冲和加载日志 |
| evidence/restored_20261104/ | 原控制版恢复、Flash 回读、JTAG 帧前进和串口状态 |

11 月 3 日抓放通过串口注入坐标完成，第一轮有用户确认 HDMI 画框。11 月 4 日没有重新确认冷启动、抓放或实屏显示；该日记录 objects=0、ROI mode=0，STATUS 接收 0 字节。自主视觉到机构闭环仍待整体验证。

`docs/` 是开发板和 AN5641 相机资料。扩展板的供电、舵机 PWM 与电机接口按实物核对，当前没有收集到 PCB 或机械 CAD 源文件。
