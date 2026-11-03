# HLS 特征核参考快照

`src/vision_feature_accel.cpp` 实现特征计算，`background_lut.h` 是冻结背景表，`tb/test_vision_feature_accel.cpp` 读取测试图。`run.tcl` 是本目录的构建入口。

从包根运行 `build/README.md` 中的 feature 命令。上层入口将本目录复制到新输出目录，并把 `data/feature_ppm/` 的 18 张实际引用图片复制到 testdata。

当前 C 文件是后续参考快照，没有与历史已导出 IP 建立逐字来源对应。重跑会产生新候选；主工程重建使用 `src/ip/vision_feature_accel/` 的冻结 RTL。历史综合和导出记录在 `sim/hls_feature/`。
