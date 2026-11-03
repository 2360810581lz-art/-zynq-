# 验证入口与结果

| 路径 | 内容 |
| --- | --- |
| hls_object_color/ | 独立颜色核 host16、RTL6、综合和导出记录 |
| hls_feature/ | 历史特征核综合与导出记录 |
| calibration/ | 三版本差分输入、轨迹及可移植核查入口 |
| skill_validation/ | 技能测试、七组回放和证据完整性结果 |
| verify_package.py | 文件名、清单、镜像配套、源码锁和 IP 依赖检查 |

在包根先执行 `python -B sim/verify_package.py`，再运行根 README 中的测试命令。历史仿真报告与本次离线检查分别保存；本次没有重新运行 HLS 或整机综合。
