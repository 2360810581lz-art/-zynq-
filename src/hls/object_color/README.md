# HLS 实例颜色核

本核按最终实例的成员像素统计颜色。成员包先输入，随后输入 BGR 光栅，输出按 slot 和 frame token 关联几何结果。颜色判定采用整数 HSV 和精确投票，shape 固定为 UNK。

`color.cpp/.h` 是综合入口，`nn_color*.h` 提供整数规则与表，`test_color.cpp` 读取 `cases.txt` 指定的 15 个二进制输入，另测试 1 个无效尺寸事务。`oracle.json` 保存独立参考结果。冻结输入已备齐，不需要额外下载训练集。

从包根按 `build/README.md` 的 HLS 步骤运行。未设置 `COLOR_RTL_TEST` 时执行完整 C 仿真；设置为 1 时执行 RTL 子集、综合与导出。

历史结果见 `sim/hls_object_color/`：host16、RTL6 通过；LUT 2678、FF 1906、DSP 2、BRAM18 2，目标周期 10 ns，估计周期 8.321 ns，光栅 II=2。这是独立核结果，整合后还需重新评估数据搬运、资源和端到端延迟。
