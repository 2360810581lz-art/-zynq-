// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.1 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XVISION_FEATURE_ACCEL_H
#define XVISION_FEATURE_ACCEL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xvision_feature_accel_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 Control_BaseAddress;
} XVision_feature_accel_Config;
#endif

typedef struct {
    u32 Control_BaseAddress;
    u32 IsReady;
} XVision_feature_accel;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XVision_feature_accel_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XVision_feature_accel_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XVision_feature_accel_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XVision_feature_accel_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XVision_feature_accel_Initialize(XVision_feature_accel *InstancePtr, u16 DeviceId);
XVision_feature_accel_Config* XVision_feature_accel_LookupConfig(u16 DeviceId);
int XVision_feature_accel_CfgInitialize(XVision_feature_accel *InstancePtr, XVision_feature_accel_Config *ConfigPtr);
#else
int XVision_feature_accel_Initialize(XVision_feature_accel *InstancePtr, const char* InstanceName);
int XVision_feature_accel_Release(XVision_feature_accel *InstancePtr);
#endif

void XVision_feature_accel_Start(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_IsDone(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_IsIdle(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_IsReady(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_EnableAutoRestart(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_DisableAutoRestart(XVision_feature_accel *InstancePtr);

void XVision_feature_accel_Set_roi_x0_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_x0_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_y0_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_y0_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_x1_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_x1_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_y1_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_y1_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_detection_padding_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_detection_padding_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_valid_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_valid_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_tl_x_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_tl_x_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_tl_y_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_tl_y_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_tr_x_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_tr_x_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_tr_y_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_tr_y_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_br_x_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_br_x_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_br_y_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_br_y_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_bl_x_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_bl_x_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_roi_bl_y_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_roi_bl_y_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_background_delta_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_background_delta_V(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_Set_min_saturation_V(XVision_feature_accel *InstancePtr, u32 Data);
u32 XVision_feature_accel_Get_min_saturation_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_detected_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_detected_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_area_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_area_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_center_x_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_center_x_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_center_y_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_center_y_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_bbox_x0_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_bbox_x0_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_bbox_y0_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_bbox_y0_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_bbox_x1_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_bbox_x1_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_bbox_y1_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_bbox_y1_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_mean_hue_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_mean_hue_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_color_id_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_color_id_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_shape_id_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_shape_id_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_class_id_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_class_id_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_fill_q8_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_fill_q8_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_compactness_q12_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_compactness_q12_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_inertia_q20_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_inertia_q20_V_vld(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_luma_variance_V(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_Get_luma_variance_V_vld(XVision_feature_accel *InstancePtr);

void XVision_feature_accel_InterruptGlobalEnable(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_InterruptGlobalDisable(XVision_feature_accel *InstancePtr);
void XVision_feature_accel_InterruptEnable(XVision_feature_accel *InstancePtr, u32 Mask);
void XVision_feature_accel_InterruptDisable(XVision_feature_accel *InstancePtr, u32 Mask);
void XVision_feature_accel_InterruptClear(XVision_feature_accel *InstancePtr, u32 Mask);
u32 XVision_feature_accel_InterruptGetEnabled(XVision_feature_accel *InstancePtr);
u32 XVision_feature_accel_InterruptGetStatus(XVision_feature_accel *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
