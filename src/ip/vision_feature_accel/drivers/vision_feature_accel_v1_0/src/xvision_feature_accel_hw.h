// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.1 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// control
// 0x000 : Control signals
//         bit 0  - ap_start (Read/Write/COH)
//         bit 1  - ap_done (Read/COR)
//         bit 2  - ap_idle (Read)
//         bit 3  - ap_ready (Read)
//         bit 7  - auto_restart (Read/Write)
//         others - reserved
// 0x004 : Global Interrupt Enable Register
//         bit 0  - Global Interrupt Enable (Read/Write)
//         others - reserved
// 0x008 : IP Interrupt Enable Register (Read/Write)
//         bit 0  - Channel 0 (ap_done)
//         bit 1  - Channel 1 (ap_ready)
//         others - reserved
// 0x00c : IP Interrupt Status Register (Read/TOW)
//         bit 0  - Channel 0 (ap_done)
//         bit 1  - Channel 1 (ap_ready)
//         others - reserved
// 0x010 : Data signal of roi_x0_V
//         bit 15~0 - roi_x0_V[15:0] (Read/Write)
//         others   - reserved
// 0x014 : reserved
// 0x018 : Data signal of roi_y0_V
//         bit 15~0 - roi_y0_V[15:0] (Read/Write)
//         others   - reserved
// 0x01c : reserved
// 0x020 : Data signal of roi_x1_V
//         bit 15~0 - roi_x1_V[15:0] (Read/Write)
//         others   - reserved
// 0x024 : reserved
// 0x028 : Data signal of roi_y1_V
//         bit 15~0 - roi_y1_V[15:0] (Read/Write)
//         others   - reserved
// 0x02c : reserved
// 0x030 : Data signal of detection_padding_V
//         bit 7~0 - detection_padding_V[7:0] (Read/Write)
//         others  - reserved
// 0x034 : reserved
// 0x038 : Data signal of roi_valid_V
//         bit 0  - roi_valid_V[0] (Read/Write)
//         others - reserved
// 0x03c : reserved
// 0x040 : Data signal of roi_tl_x_V
//         bit 15~0 - roi_tl_x_V[15:0] (Read/Write)
//         others   - reserved
// 0x044 : reserved
// 0x048 : Data signal of roi_tl_y_V
//         bit 15~0 - roi_tl_y_V[15:0] (Read/Write)
//         others   - reserved
// 0x04c : reserved
// 0x050 : Data signal of roi_tr_x_V
//         bit 15~0 - roi_tr_x_V[15:0] (Read/Write)
//         others   - reserved
// 0x054 : reserved
// 0x058 : Data signal of roi_tr_y_V
//         bit 15~0 - roi_tr_y_V[15:0] (Read/Write)
//         others   - reserved
// 0x05c : reserved
// 0x060 : Data signal of roi_br_x_V
//         bit 15~0 - roi_br_x_V[15:0] (Read/Write)
//         others   - reserved
// 0x064 : reserved
// 0x068 : Data signal of roi_br_y_V
//         bit 15~0 - roi_br_y_V[15:0] (Read/Write)
//         others   - reserved
// 0x06c : reserved
// 0x070 : Data signal of roi_bl_x_V
//         bit 15~0 - roi_bl_x_V[15:0] (Read/Write)
//         others   - reserved
// 0x074 : reserved
// 0x078 : Data signal of roi_bl_y_V
//         bit 15~0 - roi_bl_y_V[15:0] (Read/Write)
//         others   - reserved
// 0x07c : reserved
// 0x080 : Data signal of background_delta_V
//         bit 7~0 - background_delta_V[7:0] (Read/Write)
//         others  - reserved
// 0x084 : reserved
// 0x088 : Data signal of min_saturation_V
//         bit 7~0 - min_saturation_V[7:0] (Read/Write)
//         others  - reserved
// 0x08c : reserved
// 0x090 : Data signal of detected_V
//         bit 0  - detected_V[0] (Read)
//         others - reserved
// 0x094 : Control signal of detected_V
//         bit 0  - detected_V_ap_vld (Read/COR)
//         others - reserved
// 0x098 : Data signal of area_V
//         bit 31~0 - area_V[31:0] (Read)
// 0x09c : Control signal of area_V
//         bit 0  - area_V_ap_vld (Read/COR)
//         others - reserved
// 0x0a0 : Data signal of center_x_V
//         bit 15~0 - center_x_V[15:0] (Read)
//         others   - reserved
// 0x0a4 : Control signal of center_x_V
//         bit 0  - center_x_V_ap_vld (Read/COR)
//         others - reserved
// 0x0a8 : Data signal of center_y_V
//         bit 15~0 - center_y_V[15:0] (Read)
//         others   - reserved
// 0x0ac : Control signal of center_y_V
//         bit 0  - center_y_V_ap_vld (Read/COR)
//         others - reserved
// 0x0b0 : Data signal of bbox_x0_V
//         bit 15~0 - bbox_x0_V[15:0] (Read)
//         others   - reserved
// 0x0b4 : Control signal of bbox_x0_V
//         bit 0  - bbox_x0_V_ap_vld (Read/COR)
//         others - reserved
// 0x0b8 : Data signal of bbox_y0_V
//         bit 15~0 - bbox_y0_V[15:0] (Read)
//         others   - reserved
// 0x0bc : Control signal of bbox_y0_V
//         bit 0  - bbox_y0_V_ap_vld (Read/COR)
//         others - reserved
// 0x0c0 : Data signal of bbox_x1_V
//         bit 15~0 - bbox_x1_V[15:0] (Read)
//         others   - reserved
// 0x0c4 : Control signal of bbox_x1_V
//         bit 0  - bbox_x1_V_ap_vld (Read/COR)
//         others - reserved
// 0x0c8 : Data signal of bbox_y1_V
//         bit 15~0 - bbox_y1_V[15:0] (Read)
//         others   - reserved
// 0x0cc : Control signal of bbox_y1_V
//         bit 0  - bbox_y1_V_ap_vld (Read/COR)
//         others - reserved
// 0x0d0 : Data signal of mean_hue_V
//         bit 7~0 - mean_hue_V[7:0] (Read)
//         others  - reserved
// 0x0d4 : Control signal of mean_hue_V
//         bit 0  - mean_hue_V_ap_vld (Read/COR)
//         others - reserved
// 0x0d8 : Data signal of color_id_V
//         bit 3~0 - color_id_V[3:0] (Read)
//         others  - reserved
// 0x0dc : Control signal of color_id_V
//         bit 0  - color_id_V_ap_vld (Read/COR)
//         others - reserved
// 0x0e0 : Data signal of shape_id_V
//         bit 3~0 - shape_id_V[3:0] (Read)
//         others  - reserved
// 0x0e4 : Control signal of shape_id_V
//         bit 0  - shape_id_V_ap_vld (Read/COR)
//         others - reserved
// 0x0e8 : Data signal of class_id_V
//         bit 7~0 - class_id_V[7:0] (Read)
//         others  - reserved
// 0x0ec : Control signal of class_id_V
//         bit 0  - class_id_V_ap_vld (Read/COR)
//         others - reserved
// 0x0f0 : Data signal of fill_q8_V
//         bit 15~0 - fill_q8_V[15:0] (Read)
//         others   - reserved
// 0x0f4 : Control signal of fill_q8_V
//         bit 0  - fill_q8_V_ap_vld (Read/COR)
//         others - reserved
// 0x0f8 : Data signal of compactness_q12_V
//         bit 15~0 - compactness_q12_V[15:0] (Read)
//         others   - reserved
// 0x0fc : Control signal of compactness_q12_V
//         bit 0  - compactness_q12_V_ap_vld (Read/COR)
//         others - reserved
// 0x100 : Data signal of inertia_q20_V
//         bit 31~0 - inertia_q20_V[31:0] (Read)
// 0x104 : Control signal of inertia_q20_V
//         bit 0  - inertia_q20_V_ap_vld (Read/COR)
//         others - reserved
// 0x108 : Data signal of luma_variance_V
//         bit 31~0 - luma_variance_V[31:0] (Read)
// 0x10c : Control signal of luma_variance_V
//         bit 0  - luma_variance_V_ap_vld (Read/COR)
//         others - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_AP_CTRL                  0x000
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_GIE                      0x004
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_IER                      0x008
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ISR                      0x00c
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_X0_V_DATA            0x010
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_X0_V_DATA            16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_Y0_V_DATA            0x018
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_Y0_V_DATA            16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_X1_V_DATA            0x020
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_X1_V_DATA            16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_Y1_V_DATA            0x028
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_Y1_V_DATA            16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_DETECTION_PADDING_V_DATA 0x030
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_DETECTION_PADDING_V_DATA 8
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_VALID_V_DATA         0x038
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_VALID_V_DATA         1
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TL_X_V_DATA          0x040
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_TL_X_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TL_Y_V_DATA          0x048
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_TL_Y_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TR_X_V_DATA          0x050
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_TR_X_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_TR_Y_V_DATA          0x058
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_TR_Y_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BR_X_V_DATA          0x060
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_BR_X_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BR_Y_V_DATA          0x068
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_BR_Y_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BL_X_V_DATA          0x070
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_BL_X_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_ROI_BL_Y_V_DATA          0x078
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_ROI_BL_Y_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BACKGROUND_DELTA_V_DATA  0x080
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_BACKGROUND_DELTA_V_DATA  8
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_MIN_SATURATION_V_DATA    0x088
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_MIN_SATURATION_V_DATA    8
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_DETECTED_V_DATA          0x090
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_DETECTED_V_DATA          1
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_DETECTED_V_CTRL          0x094
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_AREA_V_DATA              0x098
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_AREA_V_DATA              32
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_AREA_V_CTRL              0x09c
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_CENTER_X_V_DATA          0x0a0
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_CENTER_X_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_CENTER_X_V_CTRL          0x0a4
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_CENTER_Y_V_DATA          0x0a8
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_CENTER_Y_V_DATA          16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_CENTER_Y_V_CTRL          0x0ac
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_X0_V_DATA           0x0b0
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_BBOX_X0_V_DATA           16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_X0_V_CTRL           0x0b4
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_Y0_V_DATA           0x0b8
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_BBOX_Y0_V_DATA           16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_Y0_V_CTRL           0x0bc
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_X1_V_DATA           0x0c0
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_BBOX_X1_V_DATA           16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_X1_V_CTRL           0x0c4
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_Y1_V_DATA           0x0c8
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_BBOX_Y1_V_DATA           16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_BBOX_Y1_V_CTRL           0x0cc
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_MEAN_HUE_V_DATA          0x0d0
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_MEAN_HUE_V_DATA          8
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_MEAN_HUE_V_CTRL          0x0d4
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_COLOR_ID_V_DATA          0x0d8
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_COLOR_ID_V_DATA          4
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_COLOR_ID_V_CTRL          0x0dc
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_SHAPE_ID_V_DATA          0x0e0
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_SHAPE_ID_V_DATA          4
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_SHAPE_ID_V_CTRL          0x0e4
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_CLASS_ID_V_DATA          0x0e8
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_CLASS_ID_V_DATA          8
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_CLASS_ID_V_CTRL          0x0ec
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_FILL_Q8_V_DATA           0x0f0
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_FILL_Q8_V_DATA           16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_FILL_Q8_V_CTRL           0x0f4
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_COMPACTNESS_Q12_V_DATA   0x0f8
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_COMPACTNESS_Q12_V_DATA   16
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_COMPACTNESS_Q12_V_CTRL   0x0fc
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_INERTIA_Q20_V_DATA       0x100
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_INERTIA_Q20_V_DATA       32
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_INERTIA_Q20_V_CTRL       0x104
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_LUMA_VARIANCE_V_DATA     0x108
#define XVISION_FEATURE_ACCEL_CONTROL_BITS_LUMA_VARIANCE_V_DATA     32
#define XVISION_FEATURE_ACCEL_CONTROL_ADDR_LUMA_VARIANCE_V_CTRL     0x10c

