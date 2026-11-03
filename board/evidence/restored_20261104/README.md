# 11 月 4 日原控制版恢复

`source_restore.json` 记录恢复来源，`image_verification.json` 和 `verification.log` 记录镜像核对，`sdk_commit_*/` 是 Flash 提交日志，`board_*/` 是 JTAG 加载与运行记录。发布文件在 `board/release/`，控制源码锁在 `report/provenance/restore_manifest.json`。

Flash 全量回读一致，JTAG 下观察到帧前进。此次没有重新验收冷启动、HDMI 实屏或机械抓放。串口 STATUS 接收为 0 字节，记录中 objects=0、ROI mode=0；本目录不含新运动测试。
