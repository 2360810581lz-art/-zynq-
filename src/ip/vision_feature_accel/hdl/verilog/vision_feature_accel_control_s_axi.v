// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.1 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
`timescale 1ns/1ps
module vision_feature_accel_control_s_axi
#(parameter
    C_S_AXI_ADDR_WIDTH = 9,
    C_S_AXI_DATA_WIDTH = 32
)(
    input  wire                          ACLK,
    input  wire                          ARESET,
    input  wire                          ACLK_EN,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] AWADDR,
    input  wire                          AWVALID,
    output wire                          AWREADY,
    input  wire [C_S_AXI_DATA_WIDTH-1:0] WDATA,
    input  wire [C_S_AXI_DATA_WIDTH/8-1:0] WSTRB,
    input  wire                          WVALID,
    output wire                          WREADY,
    output wire [1:0]                    BRESP,
    output wire                          BVALID,
    input  wire                          BREADY,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] ARADDR,
    input  wire                          ARVALID,
    output wire                          ARREADY,
    output wire [C_S_AXI_DATA_WIDTH-1:0] RDATA,
    output wire [1:0]                    RRESP,
    output wire                          RVALID,
    input  wire                          RREADY,
    output wire                          interrupt,
    output wire                          ap_start,
    input  wire                          ap_done,
    input  wire                          ap_ready,
    input  wire                          ap_idle,
    output wire [15:0]                   roi_x0_V,
    output wire [15:0]                   roi_y0_V,
    output wire [15:0]                   roi_x1_V,
    output wire [15:0]                   roi_y1_V,
    output wire [7:0]                    detection_padding_V,
    output wire [0:0]                    roi_valid_V,
    output wire [15:0]                   roi_tl_x_V,
    output wire [15:0]                   roi_tl_y_V,
    output wire [15:0]                   roi_tr_x_V,
    output wire [15:0]                   roi_tr_y_V,
    output wire [15:0]                   roi_br_x_V,
    output wire [15:0]                   roi_br_y_V,
    output wire [15:0]                   roi_bl_x_V,
    output wire [15:0]                   roi_bl_y_V,
    output wire [7:0]                    background_delta_V,
    output wire [7:0]                    min_saturation_V,
    input  wire [0:0]                    detected_V,
    input  wire                          detected_V_ap_vld,
    input  wire [31:0]                   area_V,
    input  wire                          area_V_ap_vld,
    input  wire [15:0]                   center_x_V,
    input  wire                          center_x_V_ap_vld,
    input  wire [15:0]                   center_y_V,
    input  wire                          center_y_V_ap_vld,
    input  wire [15:0]                   bbox_x0_V,
    input  wire                          bbox_x0_V_ap_vld,
    input  wire [15:0]                   bbox_y0_V,
    input  wire                          bbox_y0_V_ap_vld,
    input  wire [15:0]                   bbox_x1_V,
    input  wire                          bbox_x1_V_ap_vld,
    input  wire [15:0]                   bbox_y1_V,
    input  wire                          bbox_y1_V_ap_vld,
    input  wire [7:0]                    mean_hue_V,
    input  wire                          mean_hue_V_ap_vld,
    input  wire [3:0]                    color_id_V,
    input  wire                          color_id_V_ap_vld,
    input  wire [3:0]                    shape_id_V,
    input  wire                          shape_id_V_ap_vld,
    input  wire [7:0]                    class_id_V,
    input  wire                          class_id_V_ap_vld,
    input  wire [15:0]                   fill_q8_V,
    input  wire                          fill_q8_V_ap_vld,
    input  wire [15:0]                   compactness_q12_V,
    input  wire                          compactness_q12_V_ap_vld,
    input  wire [31:0]                   inertia_q20_V,
    input  wire                          inertia_q20_V_ap_vld,
    input  wire [31:0]                   luma_variance_V,
    input  wire                          luma_variance_V_ap_vld
);
//------------------------Address Info-------------------
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

//------------------------Parameter----------------------
localparam
    ADDR_AP_CTRL                    = 9'h000,
    ADDR_GIE                        = 9'h004,
    ADDR_IER                        = 9'h008,
    ADDR_ISR                        = 9'h00c,
    ADDR_ROI_X0_V_DATA_0            = 9'h010,
    ADDR_ROI_X0_V_CTRL              = 9'h014,
    ADDR_ROI_Y0_V_DATA_0            = 9'h018,
    ADDR_ROI_Y0_V_CTRL              = 9'h01c,
    ADDR_ROI_X1_V_DATA_0            = 9'h020,
    ADDR_ROI_X1_V_CTRL              = 9'h024,
    ADDR_ROI_Y1_V_DATA_0            = 9'h028,
    ADDR_ROI_Y1_V_CTRL              = 9'h02c,
    ADDR_DETECTION_PADDING_V_DATA_0 = 9'h030,
    ADDR_DETECTION_PADDING_V_CTRL   = 9'h034,
    ADDR_ROI_VALID_V_DATA_0         = 9'h038,
    ADDR_ROI_VALID_V_CTRL           = 9'h03c,
    ADDR_ROI_TL_X_V_DATA_0          = 9'h040,
    ADDR_ROI_TL_X_V_CTRL            = 9'h044,
    ADDR_ROI_TL_Y_V_DATA_0          = 9'h048,
    ADDR_ROI_TL_Y_V_CTRL            = 9'h04c,
    ADDR_ROI_TR_X_V_DATA_0          = 9'h050,
    ADDR_ROI_TR_X_V_CTRL            = 9'h054,
    ADDR_ROI_TR_Y_V_DATA_0          = 9'h058,
    ADDR_ROI_TR_Y_V_CTRL            = 9'h05c,
    ADDR_ROI_BR_X_V_DATA_0          = 9'h060,
    ADDR_ROI_BR_X_V_CTRL            = 9'h064,
    ADDR_ROI_BR_Y_V_DATA_0          = 9'h068,
    ADDR_ROI_BR_Y_V_CTRL            = 9'h06c,
    ADDR_ROI_BL_X_V_DATA_0          = 9'h070,
    ADDR_ROI_BL_X_V_CTRL            = 9'h074,
    ADDR_ROI_BL_Y_V_DATA_0          = 9'h078,
    ADDR_ROI_BL_Y_V_CTRL            = 9'h07c,
    ADDR_BACKGROUND_DELTA_V_DATA_0  = 9'h080,
    ADDR_BACKGROUND_DELTA_V_CTRL    = 9'h084,
    ADDR_MIN_SATURATION_V_DATA_0    = 9'h088,
    ADDR_MIN_SATURATION_V_CTRL      = 9'h08c,
    ADDR_DETECTED_V_DATA_0          = 9'h090,
    ADDR_DETECTED_V_CTRL            = 9'h094,
    ADDR_AREA_V_DATA_0              = 9'h098,
    ADDR_AREA_V_CTRL                = 9'h09c,
    ADDR_CENTER_X_V_DATA_0          = 9'h0a0,
    ADDR_CENTER_X_V_CTRL            = 9'h0a4,
    ADDR_CENTER_Y_V_DATA_0          = 9'h0a8,
    ADDR_CENTER_Y_V_CTRL            = 9'h0ac,
    ADDR_BBOX_X0_V_DATA_0           = 9'h0b0,
    ADDR_BBOX_X0_V_CTRL             = 9'h0b4,
    ADDR_BBOX_Y0_V_DATA_0           = 9'h0b8,
    ADDR_BBOX_Y0_V_CTRL             = 9'h0bc,
    ADDR_BBOX_X1_V_DATA_0           = 9'h0c0,
    ADDR_BBOX_X1_V_CTRL             = 9'h0c4,
    ADDR_BBOX_Y1_V_DATA_0           = 9'h0c8,
    ADDR_BBOX_Y1_V_CTRL             = 9'h0cc,
    ADDR_MEAN_HUE_V_DATA_0          = 9'h0d0,
    ADDR_MEAN_HUE_V_CTRL            = 9'h0d4,
    ADDR_COLOR_ID_V_DATA_0          = 9'h0d8,
    ADDR_COLOR_ID_V_CTRL            = 9'h0dc,
    ADDR_SHAPE_ID_V_DATA_0          = 9'h0e0,
    ADDR_SHAPE_ID_V_CTRL            = 9'h0e4,
    ADDR_CLASS_ID_V_DATA_0          = 9'h0e8,
    ADDR_CLASS_ID_V_CTRL            = 9'h0ec,
    ADDR_FILL_Q8_V_DATA_0           = 9'h0f0,
    ADDR_FILL_Q8_V_CTRL             = 9'h0f4,
    ADDR_COMPACTNESS_Q12_V_DATA_0   = 9'h0f8,
    ADDR_COMPACTNESS_Q12_V_CTRL     = 9'h0fc,
    ADDR_INERTIA_Q20_V_DATA_0       = 9'h100,
    ADDR_INERTIA_Q20_V_CTRL         = 9'h104,
    ADDR_LUMA_VARIANCE_V_DATA_0     = 9'h108,
    ADDR_LUMA_VARIANCE_V_CTRL       = 9'h10c,
    WRIDLE                          = 2'd0,
    WRDATA                          = 2'd1,
    WRRESP                          = 2'd2,
    WRRESET                         = 2'd3,
    RDIDLE                          = 2'd0,
    RDDATA                          = 2'd1,
    RDRESET                         = 2'd2,
    ADDR_BITS         = 9;

//------------------------Local signal-------------------
    reg  [1:0]                    wstate = WRRESET;
    reg  [1:0]                    wnext;
    reg  [ADDR_BITS-1:0]          waddr;
    wire [31:0]                   wmask;
    wire                          aw_hs;
    wire                          w_hs;
    reg  [1:0]                    rstate = RDRESET;
    reg  [1:0]                    rnext;
    reg  [31:0]                   rdata;
    wire                          ar_hs;
    wire [ADDR_BITS-1:0]          raddr;
    // internal registers
    reg                           int_ap_idle;
    reg                           int_ap_ready;
    reg                           int_ap_done = 1'b0;
    reg                           int_ap_start = 1'b0;
    reg                           int_auto_restart = 1'b0;
    reg                           int_gie = 1'b0;
    reg  [1:0]                    int_ier = 2'b0;
    reg  [1:0]                    int_isr = 2'b0;
    reg  [15:0]                   int_roi_x0_V = 'b0;
    reg  [15:0]                   int_roi_y0_V = 'b0;
    reg  [15:0]                   int_roi_x1_V = 'b0;
    reg  [15:0]                   int_roi_y1_V = 'b0;
    reg  [7:0]                    int_detection_padding_V = 'b0;
    reg  [0:0]                    int_roi_valid_V = 'b0;
    reg  [15:0]                   int_roi_tl_x_V = 'b0;
    reg  [15:0]                   int_roi_tl_y_V = 'b0;
    reg  [15:0]                   int_roi_tr_x_V = 'b0;
    reg  [15:0]                   int_roi_tr_y_V = 'b0;
    reg  [15:0]                   int_roi_br_x_V = 'b0;
    reg  [15:0]                   int_roi_br_y_V = 'b0;
    reg  [15:0]                   int_roi_bl_x_V = 'b0;
    reg  [15:0]                   int_roi_bl_y_V = 'b0;
    reg  [7:0]                    int_background_delta_V = 'b0;
    reg  [7:0]                    int_min_saturation_V = 'b0;
    reg  [0:0]                    int_detected_V = 'b0;
    reg                           int_detected_V_ap_vld;
    reg  [31:0]                   int_area_V = 'b0;
    reg                           int_area_V_ap_vld;
    reg  [15:0]                   int_center_x_V = 'b0;
    reg                           int_center_x_V_ap_vld;
    reg  [15:0]                   int_center_y_V = 'b0;
    reg                           int_center_y_V_ap_vld;
    reg  [15:0]                   int_bbox_x0_V = 'b0;
    reg                           int_bbox_x0_V_ap_vld;
    reg  [15:0]                   int_bbox_y0_V = 'b0;
    reg                           int_bbox_y0_V_ap_vld;
    reg  [15:0]                   int_bbox_x1_V = 'b0;
    reg                           int_bbox_x1_V_ap_vld;
    reg  [15:0]                   int_bbox_y1_V = 'b0;
    reg                           int_bbox_y1_V_ap_vld;
    reg  [7:0]                    int_mean_hue_V = 'b0;
    reg                           int_mean_hue_V_ap_vld;
    reg  [3:0]                    int_color_id_V = 'b0;
    reg                           int_color_id_V_ap_vld;
    reg  [3:0]                    int_shape_id_V = 'b0;
    reg                           int_shape_id_V_ap_vld;
    reg  [7:0]                    int_class_id_V = 'b0;
    reg                           int_class_id_V_ap_vld;
    reg  [15:0]                   int_fill_q8_V = 'b0;
    reg                           int_fill_q8_V_ap_vld;
    reg  [15:0]                   int_compactness_q12_V = 'b0;
    reg                           int_compactness_q12_V_ap_vld;
    reg  [31:0]                   int_inertia_q20_V = 'b0;
    reg                           int_inertia_q20_V_ap_vld;
    reg  [31:0]                   int_luma_variance_V = 'b0;
    reg                           int_luma_variance_V_ap_vld;

//------------------------Instantiation------------------

//------------------------AXI write fsm------------------
assign AWREADY = (wstate == WRIDLE);
assign WREADY  = (wstate == WRDATA);
assign BRESP   = 2'b00;  // OKAY
assign BVALID  = (wstate == WRRESP);
assign wmask   = { {8{WSTRB[3]}}, {8{WSTRB[2]}}, {8{WSTRB[1]}}, {8{WSTRB[0]}} };
assign aw_hs   = AWVALID & AWREADY;
assign w_hs    = WVALID & WREADY;

// wstate
always @(posedge ACLK) begin
    if (ARESET)
        wstate <= WRRESET;
    else if (ACLK_EN)
        wstate <= wnext;
end

// wnext
always @(*) begin
    case (wstate)
        WRIDLE:
            if (AWVALID)
                wnext = WRDATA;
            else
                wnext = WRIDLE;
        WRDATA:
            if (WVALID)
                wnext = WRRESP;
            else
                wnext = WRDATA;
        WRRESP:
            if (BREADY)
                wnext = WRIDLE;
            else
                wnext = WRRESP;
        default:
            wnext = WRIDLE;
    endcase
end

// waddr
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (aw_hs)
            waddr <= AWADDR[ADDR_BITS-1:0];
    end
end

//------------------------AXI read fsm-------------------
assign ARREADY = (rstate == RDIDLE);
assign RDATA   = rdata;
assign RRESP   = 2'b00;  // OKAY
assign RVALID  = (rstate == RDDATA);
assign ar_hs   = ARVALID & ARREADY;
assign raddr   = ARADDR[ADDR_BITS-1:0];

// rstate
always @(posedge ACLK) begin
    if (ARESET)
        rstate <= RDRESET;
    else if (ACLK_EN)
        rstate <= rnext;
end

// rnext
always @(*) begin
    case (rstate)
        RDIDLE:
            if (ARVALID)
                rnext = RDDATA;
            else
                rnext = RDIDLE;
        RDDATA:
            if (RREADY & RVALID)
                rnext = RDIDLE;
            else
                rnext = RDDATA;
        default:
            rnext = RDIDLE;
    endcase
end

// rdata
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (ar_hs) begin
            rdata <= 1'b0;
            case (raddr)
                ADDR_AP_CTRL: begin
                    rdata[0] <= int_ap_start;
                    rdata[1] <= int_ap_done;
                    rdata[2] <= int_ap_idle;
                    rdata[3] <= int_ap_ready;
                    rdata[7] <= int_auto_restart;
                end
                ADDR_GIE: begin
                    rdata <= int_gie;
                end
                ADDR_IER: begin
                    rdata <= int_ier;
                end
                ADDR_ISR: begin
                    rdata <= int_isr;
                end
                ADDR_ROI_X0_V_DATA_0: begin
                    rdata <= int_roi_x0_V[15:0];
                end
                ADDR_ROI_Y0_V_DATA_0: begin
                    rdata <= int_roi_y0_V[15:0];
                end
                ADDR_ROI_X1_V_DATA_0: begin
                    rdata <= int_roi_x1_V[15:0];
                end
                ADDR_ROI_Y1_V_DATA_0: begin
                    rdata <= int_roi_y1_V[15:0];
                end
                ADDR_DETECTION_PADDING_V_DATA_0: begin
                    rdata <= int_detection_padding_V[7:0];
                end
                ADDR_ROI_VALID_V_DATA_0: begin
                    rdata <= int_roi_valid_V[0:0];
                end
                ADDR_ROI_TL_X_V_DATA_0: begin
                    rdata <= int_roi_tl_x_V[15:0];
                end
                ADDR_ROI_TL_Y_V_DATA_0: begin
                    rdata <= int_roi_tl_y_V[15:0];
                end
                ADDR_ROI_TR_X_V_DATA_0: begin
                    rdata <= int_roi_tr_x_V[15:0];
                end
                ADDR_ROI_TR_Y_V_DATA_0: begin
                    rdata <= int_roi_tr_y_V[15:0];
                end
                ADDR_ROI_BR_X_V_DATA_0: begin
                    rdata <= int_roi_br_x_V[15:0];
                end
                ADDR_ROI_BR_Y_V_DATA_0: begin
                    rdata <= int_roi_br_y_V[15:0];
                end
                ADDR_ROI_BL_X_V_DATA_0: begin
                    rdata <= int_roi_bl_x_V[15:0];
                end
                ADDR_ROI_BL_Y_V_DATA_0: begin
                    rdata <= int_roi_bl_y_V[15:0];
                end
                ADDR_BACKGROUND_DELTA_V_DATA_0: begin
                    rdata <= int_background_delta_V[7:0];
                end
                ADDR_MIN_SATURATION_V_DATA_0: begin
                    rdata <= int_min_saturation_V[7:0];
                end
                ADDR_DETECTED_V_DATA_0: begin
                    rdata <= int_detected_V[0:0];
                end
                ADDR_DETECTED_V_CTRL: begin
                    rdata[0] <= int_detected_V_ap_vld;
                end
                ADDR_AREA_V_DATA_0: begin
                    rdata <= int_area_V[31:0];
                end
                ADDR_AREA_V_CTRL: begin
                    rdata[0] <= int_area_V_ap_vld;
                end
                ADDR_CENTER_X_V_DATA_0: begin
                    rdata <= int_center_x_V[15:0];
                end
                ADDR_CENTER_X_V_CTRL: begin
                    rdata[0] <= int_center_x_V_ap_vld;
                end
                ADDR_CENTER_Y_V_DATA_0: begin
                    rdata <= int_center_y_V[15:0];
                end
                ADDR_CENTER_Y_V_CTRL: begin
                    rdata[0] <= int_center_y_V_ap_vld;
                end
                ADDR_BBOX_X0_V_DATA_0: begin
                    rdata <= int_bbox_x0_V[15:0];
                end
                ADDR_BBOX_X0_V_CTRL: begin
                    rdata[0] <= int_bbox_x0_V_ap_vld;
                end
                ADDR_BBOX_Y0_V_DATA_0: begin
                    rdata <= int_bbox_y0_V[15:0];
                end
                ADDR_BBOX_Y0_V_CTRL: begin
                    rdata[0] <= int_bbox_y0_V_ap_vld;
                end
                ADDR_BBOX_X1_V_DATA_0: begin
                    rdata <= int_bbox_x1_V[15:0];
                end
                ADDR_BBOX_X1_V_CTRL: begin
                    rdata[0] <= int_bbox_x1_V_ap_vld;
                end
                ADDR_BBOX_Y1_V_DATA_0: begin
                    rdata <= int_bbox_y1_V[15:0];
                end
                ADDR_BBOX_Y1_V_CTRL: begin
                    rdata[0] <= int_bbox_y1_V_ap_vld;
                end
                ADDR_MEAN_HUE_V_DATA_0: begin
                    rdata <= int_mean_hue_V[7:0];
                end
                ADDR_MEAN_HUE_V_CTRL: begin
                    rdata[0] <= int_mean_hue_V_ap_vld;
                end
                ADDR_COLOR_ID_V_DATA_0: begin
                    rdata <= int_color_id_V[3:0];
                end
                ADDR_COLOR_ID_V_CTRL: begin
                    rdata[0] <= int_color_id_V_ap_vld;
                end
                ADDR_SHAPE_ID_V_DATA_0: begin
                    rdata <= int_shape_id_V[3:0];
                end
                ADDR_SHAPE_ID_V_CTRL: begin
                    rdata[0] <= int_shape_id_V_ap_vld;
                end
                ADDR_CLASS_ID_V_DATA_0: begin
                    rdata <= int_class_id_V[7:0];
                end
                ADDR_CLASS_ID_V_CTRL: begin
                    rdata[0] <= int_class_id_V_ap_vld;
                end
                ADDR_FILL_Q8_V_DATA_0: begin
                    rdata <= int_fill_q8_V[15:0];
                end
                ADDR_FILL_Q8_V_CTRL: begin
                    rdata[0] <= int_fill_q8_V_ap_vld;
                end
                ADDR_COMPACTNESS_Q12_V_DATA_0: begin
                    rdata <= int_compactness_q12_V[15:0];
                end
                ADDR_COMPACTNESS_Q12_V_CTRL: begin
                    rdata[0] <= int_compactness_q12_V_ap_vld;
                end
                ADDR_INERTIA_Q20_V_DATA_0: begin
                    rdata <= int_inertia_q20_V[31:0];
                end
                ADDR_INERTIA_Q20_V_CTRL: begin
                    rdata[0] <= int_inertia_q20_V_ap_vld;
                end
                ADDR_LUMA_VARIANCE_V_DATA_0: begin
                    rdata <= int_luma_variance_V[31:0];
                end
                ADDR_LUMA_VARIANCE_V_CTRL: begin
                    rdata[0] <= int_luma_variance_V_ap_vld;
                end
            endcase
        end
    end
end


//------------------------Register logic-----------------
assign interrupt           = int_gie & (|int_isr);
assign ap_start            = int_ap_start;
assign roi_x0_V            = int_roi_x0_V;
assign roi_y0_V            = int_roi_y0_V;
assign roi_x1_V            = int_roi_x1_V;
assign roi_y1_V            = int_roi_y1_V;
assign detection_padding_V = int_detection_padding_V;
assign roi_valid_V         = int_roi_valid_V;
assign roi_tl_x_V          = int_roi_tl_x_V;
assign roi_tl_y_V          = int_roi_tl_y_V;
assign roi_tr_x_V          = int_roi_tr_x_V;
assign roi_tr_y_V          = int_roi_tr_y_V;
assign roi_br_x_V          = int_roi_br_x_V;
assign roi_br_y_V          = int_roi_br_y_V;
assign roi_bl_x_V          = int_roi_bl_x_V;
assign roi_bl_y_V          = int_roi_bl_y_V;
assign background_delta_V  = int_background_delta_V;
assign min_saturation_V    = int_min_saturation_V;
// int_ap_start
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_start <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_AP_CTRL && WSTRB[0] && WDATA[0])
            int_ap_start <= 1'b1;
        else if (ap_ready)
            int_ap_start <= int_auto_restart; // clear on handshake/auto restart
    end
end

// int_ap_done
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_done <= 1'b0;
    else if (ACLK_EN) begin
        if (ap_done)
            int_ap_done <= 1'b1;
        else if (ar_hs && raddr == ADDR_AP_CTRL)
            int_ap_done <= 1'b0; // clear on read
    end
end

// int_ap_idle
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_idle <= 1'b0;
    else if (ACLK_EN) begin
            int_ap_idle <= ap_idle;
    end
end

// int_ap_ready
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_ready <= 1'b0;
    else if (ACLK_EN) begin
            int_ap_ready <= ap_ready;
    end
end

// int_auto_restart
always @(posedge ACLK) begin
    if (ARESET)
        int_auto_restart <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_AP_CTRL && WSTRB[0])
            int_auto_restart <=  WDATA[7];
    end
end

// int_gie
always @(posedge ACLK) begin
    if (ARESET)
        int_gie <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_GIE && WSTRB[0])
            int_gie <= WDATA[0];
    end
end

// int_ier
always @(posedge ACLK) begin
    if (ARESET)
        int_ier <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IER && WSTRB[0])
            int_ier <= WDATA[1:0];
    end
end

// int_isr[0]
always @(posedge ACLK) begin
    if (ARESET)
        int_isr[0] <= 1'b0;
    else if (ACLK_EN) begin
        if (int_ier[0] & ap_done)
            int_isr[0] <= 1'b1;
        else if (w_hs && waddr == ADDR_ISR && WSTRB[0])
            int_isr[0] <= int_isr[0] ^ WDATA[0]; // toggle on write
    end
end

// int_isr[1]
always @(posedge ACLK) begin
    if (ARESET)
        int_isr[1] <= 1'b0;
    else if (ACLK_EN) begin
        if (int_ier[1] & ap_ready)
            int_isr[1] <= 1'b1;
        else if (w_hs && waddr == ADDR_ISR && WSTRB[0])
            int_isr[1] <= int_isr[1] ^ WDATA[1]; // toggle on write
    end
end

// int_roi_x0_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_x0_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_X0_V_DATA_0)
            int_roi_x0_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_x0_V[15:0] & ~wmask);
    end
end

// int_roi_y0_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_y0_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_Y0_V_DATA_0)
            int_roi_y0_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_y0_V[15:0] & ~wmask);
    end
end

// int_roi_x1_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_x1_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_X1_V_DATA_0)
            int_roi_x1_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_x1_V[15:0] & ~wmask);
    end
end

// int_roi_y1_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_y1_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_Y1_V_DATA_0)
            int_roi_y1_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_y1_V[15:0] & ~wmask);
    end
end

// int_detection_padding_V[7:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_detection_padding_V[7:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_DETECTION_PADDING_V_DATA_0)
            int_detection_padding_V[7:0] <= (WDATA[31:0] & wmask) | (int_detection_padding_V[7:0] & ~wmask);
    end
end

// int_roi_valid_V[0:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_valid_V[0:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_VALID_V_DATA_0)
            int_roi_valid_V[0:0] <= (WDATA[31:0] & wmask) | (int_roi_valid_V[0:0] & ~wmask);
    end
end

// int_roi_tl_x_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_tl_x_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_TL_X_V_DATA_0)
            int_roi_tl_x_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_tl_x_V[15:0] & ~wmask);
    end
end

// int_roi_tl_y_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_tl_y_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_TL_Y_V_DATA_0)
            int_roi_tl_y_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_tl_y_V[15:0] & ~wmask);
    end
end

// int_roi_tr_x_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_tr_x_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_TR_X_V_DATA_0)
            int_roi_tr_x_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_tr_x_V[15:0] & ~wmask);
    end
end

// int_roi_tr_y_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_tr_y_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_TR_Y_V_DATA_0)
            int_roi_tr_y_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_tr_y_V[15:0] & ~wmask);
    end
end

// int_roi_br_x_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_br_x_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_BR_X_V_DATA_0)
            int_roi_br_x_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_br_x_V[15:0] & ~wmask);
    end
end

// int_roi_br_y_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_br_y_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_BR_Y_V_DATA_0)
            int_roi_br_y_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_br_y_V[15:0] & ~wmask);
    end
end

// int_roi_bl_x_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_bl_x_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_BL_X_V_DATA_0)
            int_roi_bl_x_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_bl_x_V[15:0] & ~wmask);
    end
end

// int_roi_bl_y_V[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_roi_bl_y_V[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROI_BL_Y_V_DATA_0)
            int_roi_bl_y_V[15:0] <= (WDATA[31:0] & wmask) | (int_roi_bl_y_V[15:0] & ~wmask);
    end
end

// int_background_delta_V[7:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_background_delta_V[7:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_BACKGROUND_DELTA_V_DATA_0)
            int_background_delta_V[7:0] <= (WDATA[31:0] & wmask) | (int_background_delta_V[7:0] & ~wmask);
    end
end

// int_min_saturation_V[7:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_min_saturation_V[7:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_MIN_SATURATION_V_DATA_0)
            int_min_saturation_V[7:0] <= (WDATA[31:0] & wmask) | (int_min_saturation_V[7:0] & ~wmask);
    end
end

// int_detected_V
always @(posedge ACLK) begin
    if (ARESET)
        int_detected_V <= 0;
    else if (ACLK_EN) begin
        if (detected_V_ap_vld)
            int_detected_V <= detected_V;
    end
end

// int_detected_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_detected_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (detected_V_ap_vld)
            int_detected_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_DETECTED_V_CTRL)
            int_detected_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_area_V
always @(posedge ACLK) begin
    if (ARESET)
        int_area_V <= 0;
    else if (ACLK_EN) begin
        if (area_V_ap_vld)
            int_area_V <= area_V;
    end
end

// int_area_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_area_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (area_V_ap_vld)
            int_area_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_AREA_V_CTRL)
            int_area_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_center_x_V
always @(posedge ACLK) begin
    if (ARESET)
        int_center_x_V <= 0;
    else if (ACLK_EN) begin
        if (center_x_V_ap_vld)
            int_center_x_V <= center_x_V;
    end
end

// int_center_x_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_center_x_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (center_x_V_ap_vld)
            int_center_x_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_CENTER_X_V_CTRL)
            int_center_x_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_center_y_V
always @(posedge ACLK) begin
    if (ARESET)
        int_center_y_V <= 0;
    else if (ACLK_EN) begin
        if (center_y_V_ap_vld)
            int_center_y_V <= center_y_V;
    end
end

// int_center_y_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_center_y_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (center_y_V_ap_vld)
            int_center_y_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_CENTER_Y_V_CTRL)
            int_center_y_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_bbox_x0_V
always @(posedge ACLK) begin
    if (ARESET)
        int_bbox_x0_V <= 0;
    else if (ACLK_EN) begin
        if (bbox_x0_V_ap_vld)
            int_bbox_x0_V <= bbox_x0_V;
    end
end

// int_bbox_x0_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_bbox_x0_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (bbox_x0_V_ap_vld)
            int_bbox_x0_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_BBOX_X0_V_CTRL)
            int_bbox_x0_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_bbox_y0_V
always @(posedge ACLK) begin
    if (ARESET)
        int_bbox_y0_V <= 0;
    else if (ACLK_EN) begin
        if (bbox_y0_V_ap_vld)
            int_bbox_y0_V <= bbox_y0_V;
    end
end

// int_bbox_y0_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_bbox_y0_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (bbox_y0_V_ap_vld)
            int_bbox_y0_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_BBOX_Y0_V_CTRL)
            int_bbox_y0_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_bbox_x1_V
always @(posedge ACLK) begin
    if (ARESET)
        int_bbox_x1_V <= 0;
    else if (ACLK_EN) begin
        if (bbox_x1_V_ap_vld)
            int_bbox_x1_V <= bbox_x1_V;
    end
end

// int_bbox_x1_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_bbox_x1_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (bbox_x1_V_ap_vld)
            int_bbox_x1_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_BBOX_X1_V_CTRL)
            int_bbox_x1_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_bbox_y1_V
always @(posedge ACLK) begin
    if (ARESET)
        int_bbox_y1_V <= 0;
    else if (ACLK_EN) begin
        if (bbox_y1_V_ap_vld)
            int_bbox_y1_V <= bbox_y1_V;
    end
end

// int_bbox_y1_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_bbox_y1_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (bbox_y1_V_ap_vld)
            int_bbox_y1_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_BBOX_Y1_V_CTRL)
            int_bbox_y1_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_mean_hue_V
always @(posedge ACLK) begin
    if (ARESET)
        int_mean_hue_V <= 0;
    else if (ACLK_EN) begin
        if (mean_hue_V_ap_vld)
            int_mean_hue_V <= mean_hue_V;
    end
end

// int_mean_hue_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_mean_hue_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (mean_hue_V_ap_vld)
            int_mean_hue_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_MEAN_HUE_V_CTRL)
            int_mean_hue_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_color_id_V
always @(posedge ACLK) begin
    if (ARESET)
        int_color_id_V <= 0;
    else if (ACLK_EN) begin
        if (color_id_V_ap_vld)
            int_color_id_V <= color_id_V;
    end
end

// int_color_id_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_color_id_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (color_id_V_ap_vld)
            int_color_id_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_COLOR_ID_V_CTRL)
            int_color_id_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_shape_id_V
always @(posedge ACLK) begin
    if (ARESET)
        int_shape_id_V <= 0;
    else if (ACLK_EN) begin
        if (shape_id_V_ap_vld)
            int_shape_id_V <= shape_id_V;
    end
end

// int_shape_id_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_shape_id_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (shape_id_V_ap_vld)
            int_shape_id_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_SHAPE_ID_V_CTRL)
            int_shape_id_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_class_id_V
always @(posedge ACLK) begin
    if (ARESET)
        int_class_id_V <= 0;
    else if (ACLK_EN) begin
        if (class_id_V_ap_vld)
            int_class_id_V <= class_id_V;
    end
end

// int_class_id_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_class_id_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (class_id_V_ap_vld)
            int_class_id_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_CLASS_ID_V_CTRL)
            int_class_id_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_fill_q8_V
always @(posedge ACLK) begin
    if (ARESET)
        int_fill_q8_V <= 0;
    else if (ACLK_EN) begin
        if (fill_q8_V_ap_vld)
            int_fill_q8_V <= fill_q8_V;
    end
end

// int_fill_q8_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_fill_q8_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (fill_q8_V_ap_vld)
            int_fill_q8_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_FILL_Q8_V_CTRL)
            int_fill_q8_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_compactness_q12_V
always @(posedge ACLK) begin
    if (ARESET)
        int_compactness_q12_V <= 0;
    else if (ACLK_EN) begin
        if (compactness_q12_V_ap_vld)
            int_compactness_q12_V <= compactness_q12_V;
    end
end

// int_compactness_q12_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_compactness_q12_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (compactness_q12_V_ap_vld)
            int_compactness_q12_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_COMPACTNESS_Q12_V_CTRL)
            int_compactness_q12_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_inertia_q20_V
always @(posedge ACLK) begin
    if (ARESET)
        int_inertia_q20_V <= 0;
    else if (ACLK_EN) begin
        if (inertia_q20_V_ap_vld)
            int_inertia_q20_V <= inertia_q20_V;
    end
end

// int_inertia_q20_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_inertia_q20_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (inertia_q20_V_ap_vld)
            int_inertia_q20_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_INERTIA_Q20_V_CTRL)
            int_inertia_q20_V_ap_vld <= 1'b0; // clear on read
    end
end

// int_luma_variance_V
always @(posedge ACLK) begin
    if (ARESET)
        int_luma_variance_V <= 0;
    else if (ACLK_EN) begin
        if (luma_variance_V_ap_vld)
            int_luma_variance_V <= luma_variance_V;
    end
end

// int_luma_variance_V_ap_vld
always @(posedge ACLK) begin
    if (ARESET)
        int_luma_variance_V_ap_vld <= 1'b0;
    else if (ACLK_EN) begin
        if (luma_variance_V_ap_vld)
            int_luma_variance_V_ap_vld <= 1'b1;
        else if (ar_hs && raddr == ADDR_LUMA_VARIANCE_V_CTRL)
            int_luma_variance_V_ap_vld <= 1'b0; // clear on read
    end
end


//------------------------Memory logic-------------------

endmodule
