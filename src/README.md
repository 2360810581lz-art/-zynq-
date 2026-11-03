# 源码说明

主工程入口是 `rtl/camera_claw_top.v`。`bd/system.bd` 连接相机、DDR、视频与显示链路，`constraints/` 保存引脚和时序约束，`ip/` 是工程引用的 IP 仓库，`ps/` 是 SDK 应用输入。19 个 PS 文件的锁定值在 `../build/ps_sources.json`。

`rtl/control/` 保留当前原控制整合版。当前控制域为 100 MHz；不要把历史 50 MHz 独立控制源码直接覆盖到这里。12 个原控制文件的哈希由 `../report/provenance/restore_manifest.json` 锁定。

`hls/object_color/` 是独立颜色候选，`hls/feature/` 是特征核 C 参考快照。主工程使用 `ip/vision_feature_accel/` 的冻结 RTL，特征核当前 C 快照不能逐字重建历史 IP。两个 HLS 工程各自的状态见对应 README。

从包根按 `../build/README.md` 创建新工程。原始 XPR、历史实验分支和工具缓存不参与这个入口。
