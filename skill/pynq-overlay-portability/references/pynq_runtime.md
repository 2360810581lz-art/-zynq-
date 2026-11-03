# PYNQ跨题迁移与分阶段运行指南

此为待硬件验证的运行模板。已验证的是离线preflight，以下流程未在本项目PYNQ板卡执行；不把SDK裸机工程称作PYNQ工程。

## 替换参数

记录目标板、SoC器件、PYNQ版本、Python版本、工具版本、时钟/复位、IP层级名、VLNV、寄存器定义、DMA位宽与最大传输长度。换算法只改契约、数据和黄金参考；换板卡须为目标器件重生成硬件。HDF与HWH不是可互换文件。

## 离线阶段

将真实同一构建的overlay.bit与overlay.hwh放在同目录。用可信构建哈希设置expected_sha256。填写required_ips，例如滤波、FFT或矩阵乘IP，寄存器名与offset来自实际HWH，不能照抄示例length=0x10。

运行preflight；失败先修输入。通过只说明本脚本能解析的元数据与契约一致。再人工核查HWH地址空间、器件信息、时钟与DMA连接。

## 板端加载与寄存器检查

以下代码仅在配套目标板环境运行，路径与IP名必须替换，不要使用examples/demo.bit。

```python
from pynq import Overlay
overlay = Overlay('/home/xilinx/overlays/my_design/my_design.bit', download=False)
print(overlay.ip_dict)       # 先核对层级名和驱动信息
# 确认板卡和产物匹配后，在需要配置硬件时调用 overlay.download()
# ip = overlay.<actual_ip_name>
# print(ip.register_map)     # 以实际HWH寄存器为准，不写死offset
```

对于ap_ctrl_hs核，只有确认控制协议、数据地址与长度寄存器后再启动；对于ap_ctrl_none流核，不套用AP_START轮询。不要向保留或只读寄存器写入。ap_done可能有读清除语义，需查实际驱动与寄存器说明。

## DMA最小黄金输入

仅当硬件设计已确认简单DMA内存到流到内存通路，且能正确产生TLAST时使用此顺序。先准备固定小输入和独立黄金输出，输出缓冲区尺寸应按实际输出字节数分配。

以下是控制流程伪代码，不能直接复制执行。完成轮询、错误检查和恢复函数必须按目标PYNQ版本实现。

```text
按实际dtype、输入和输出长度分配tx与rx
写入固定输入，初始化输出，按平台要求flush缓存
先提交接收传输，再提交发送传输
若为控制型核，在参数配置完整后启动；自由运行核跳过启动
在截止时间前轮询发送、接收以及必要的加速核状态：
    若发现错误或超时：
        保存DMA与加速核状态
        停止或复位，并确认DMA不再访问缓冲区
        将本次标记为失败，跳过结果比对
    若两通道完成且加速核状态满足契约：
        按平台要求invalidate输出缓存
        将输出与独立黄金结果逐项比较
确认DMA停止且缓冲区无其他使用者后，才释放缓冲区
```

不同PYNQ版本的完成与错误API不同，接入时必须实现有界轮询。若恢复后仍无法确认DMA停止，应保留缓冲区并按平台流程恢复，不能释放仍可能被访问的内存。先验证错误与超时分支，再测吞吐。

## 计时与失效条件

分别报告输入准备、flush、提交、硬件完成、invalidate与结果比对耗时，说明端到端测量是否包含这些步骤。记录warm-up、输入字节数和重复次数。缓存操作是否必须、缓冲区是否coherent都以目标平台为准。

不完成：器件/时钟不匹配、DMA无TLAST、传输长度超限、dtype与硬件位宽不符、不同版本驱动语义不清时，停止性能结论，保存日志与寄存器快照。未执行板端阶段的状态一律为offline_metadata_only。
