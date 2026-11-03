# 测试数据与视频素材

`feature_ppm/` 的 18 张 PPM 是特征核 testbench 实际读取的输入，保留原字节。文件名对应颜色、物体及位置，从 `build/hls_build.tcl` 的 feature 入口自动复制使用。

`samples/` 保留两组历史场景，每组一张原图和一张参考轮廓图：

| 原图 | 参考结果 |
| --- | --- |
| five_objects.png | five_objects_review.png |
| four_objects.png | four_objects_review.png |

可作为报告或视频素材，标注“历史场景原图”或“历史离线参考输出”。绿色轮廓来自当时的审核算法，不是人工像素金标，也不是当前板端识别结果。图片来源映射见 `report/provenance/source_map.json`。

颜色核的二进制输入和 oracle 位于 `src/hls/object_color/`。实际显示缓冲及板测像素比对放在 `board/evidence/`，避免与离线参考混用。
