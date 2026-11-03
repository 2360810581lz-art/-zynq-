// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.1 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xvision_feature_accel.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XVision_feature_accel_CfgInitialize(XVision_feature_accel *InstancePtr, XVision_feature_accel_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XVision_feature_accel_Start(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AP_CTRL) & 0x80;
    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XVision_feature_accel_IsDone(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XVision_feature_accel_IsIdle(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XVision_feature_accel_IsReady(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XVision_feature_accel_EnableAutoRestart(XVision_feature_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XVision_feature_accel_DisableAutoRestart(XVision_feature_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AP_CTRL, 0);
}

void XVision_feature_accel_Set_roi_x0_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_X0_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_x0_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_X0_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_y0_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_Y0_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_y0_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_Y0_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_x1_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_X1_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_x1_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_X1_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_y1_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_Y1_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_y1_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_Y1_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_detection_padding_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_DETECTION_PADDING_V_DATA, Data);
}

u32 XVision_feature_accel_Get_detection_padding_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_DETECTION_PADDING_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_valid_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_VALID_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_valid_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_VALID_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_tl_x_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TL_X_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_tl_x_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TL_X_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_tl_y_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TL_Y_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_tl_y_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TL_Y_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_tr_x_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TR_X_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_tr_x_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TR_X_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_tr_y_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TR_Y_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_tr_y_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TR_Y_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_br_x_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BR_X_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_br_x_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BR_X_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_br_y_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BR_Y_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_br_y_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BR_Y_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_bl_x_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BL_X_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_bl_x_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BL_X_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_roi_bl_y_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BL_Y_V_DATA, Data);
}

u32 XVision_feature_accel_Get_roi_bl_y_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BL_Y_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_background_delta_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BACKGROUND_DELTA_V_DATA, Data);
}

u32 XVision_feature_accel_Get_background_delta_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BACKGROUND_DELTA_V_DATA);
    return Data;
}

void XVision_feature_accel_Set_min_saturation_V(XVision_feature_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_MIN_SATURATION_V_DATA, Data);
}

u32 XVision_feature_accel_Get_min_saturation_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_MIN_SATURATION_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_detected_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_DETECTED_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_detected_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_DETECTED_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_area_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AREA_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_area_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_AREA_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_center_x_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_CENTER_X_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_center_x_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_CENTER_X_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_center_y_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_CENTER_Y_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_center_y_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_CENTER_Y_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_bbox_x0_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_X0_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_bbox_x0_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_X0_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_bbox_y0_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_Y0_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_bbox_y0_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_Y0_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_bbox_x1_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_X1_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_bbox_x1_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_X1_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_bbox_y1_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_Y1_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_bbox_y1_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_Y1_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_mean_hue_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_MEAN_HUE_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_mean_hue_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_MEAN_HUE_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_color_id_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_COLOR_ID_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_color_id_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_COLOR_ID_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_shape_id_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_SHAPE_ID_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_shape_id_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_SHAPE_ID_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_class_id_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_CLASS_ID_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_class_id_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_CLASS_ID_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_fill_q8_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_FILL_Q8_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_fill_q8_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_FILL_Q8_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_compactness_q12_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_COMPACTNESS_Q12_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_compactness_q12_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_COMPACTNESS_Q12_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_inertia_q20_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_INERTIA_Q20_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_inertia_q20_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_INERTIA_Q20_V_CTRL);
    return Data & 0x1;
}

u32 XVision_feature_accel_Get_luma_variance_V(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_LUMA_VARIANCE_V_DATA);
    return Data;
}

u32 XVision_feature_accel_Get_luma_variance_V_vld(XVision_feature_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_LUMA_VARIANCE_V_CTRL);
    return Data & 0x1;
}

void XVision_feature_accel_InterruptGlobalEnable(XVision_feature_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_GIE, 1);
}

void XVision_feature_accel_InterruptGlobalDisable(XVision_feature_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_GIE, 0);
}

void XVision_feature_accel_InterruptEnable(XVision_feature_accel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_IER);
    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_IER, Register | Mask);
}

void XVision_feature_accel_InterruptDisable(XVision_feature_accel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_IER);
    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_IER, Register & (~Mask));
}

void XVision_feature_accel_InterruptClear(XVision_feature_accel *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XVision_feature_accel_WriteReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ISR, Mask);
}

u32 XVision_feature_accel_InterruptGetEnabled(XVision_feature_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_IER);
}

u32 XVision_feature_accel_InterruptGetStatus(XVision_feature_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XVision_feature_accel_ReadReg(InstancePtr->Control_BaseAddress, XVISION_FEATURE_ACCEL_CONTROL_ADDR_ISR);
}

