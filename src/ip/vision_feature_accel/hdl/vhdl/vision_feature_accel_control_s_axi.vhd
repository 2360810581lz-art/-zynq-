-- ==============================================================
-- Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.1 (64-bit)
-- Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
-- ==============================================================
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity vision_feature_accel_control_s_axi is
generic (
    C_S_AXI_ADDR_WIDTH    : INTEGER := 9;
    C_S_AXI_DATA_WIDTH    : INTEGER := 32);
port (
    ACLK                  :in   STD_LOGIC;
    ARESET                :in   STD_LOGIC;
    ACLK_EN               :in   STD_LOGIC;
    AWADDR                :in   STD_LOGIC_VECTOR(C_S_AXI_ADDR_WIDTH-1 downto 0);
    AWVALID               :in   STD_LOGIC;
    AWREADY               :out  STD_LOGIC;
    WDATA                 :in   STD_LOGIC_VECTOR(C_S_AXI_DATA_WIDTH-1 downto 0);
    WSTRB                 :in   STD_LOGIC_VECTOR(C_S_AXI_DATA_WIDTH/8-1 downto 0);
    WVALID                :in   STD_LOGIC;
    WREADY                :out  STD_LOGIC;
    BRESP                 :out  STD_LOGIC_VECTOR(1 downto 0);
    BVALID                :out  STD_LOGIC;
    BREADY                :in   STD_LOGIC;
    ARADDR                :in   STD_LOGIC_VECTOR(C_S_AXI_ADDR_WIDTH-1 downto 0);
    ARVALID               :in   STD_LOGIC;
    ARREADY               :out  STD_LOGIC;
    RDATA                 :out  STD_LOGIC_VECTOR(C_S_AXI_DATA_WIDTH-1 downto 0);
    RRESP                 :out  STD_LOGIC_VECTOR(1 downto 0);
    RVALID                :out  STD_LOGIC;
    RREADY                :in   STD_LOGIC;
    interrupt             :out  STD_LOGIC;
    ap_start              :out  STD_LOGIC;
    ap_done               :in   STD_LOGIC;
    ap_ready              :in   STD_LOGIC;
    ap_idle               :in   STD_LOGIC;
    roi_x0_V              :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_y0_V              :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_x1_V              :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_y1_V              :out  STD_LOGIC_VECTOR(15 downto 0);
    detection_padding_V   :out  STD_LOGIC_VECTOR(7 downto 0);
    roi_valid_V           :out  STD_LOGIC_VECTOR(0 downto 0);
    roi_tl_x_V            :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_tl_y_V            :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_tr_x_V            :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_tr_y_V            :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_br_x_V            :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_br_y_V            :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_bl_x_V            :out  STD_LOGIC_VECTOR(15 downto 0);
    roi_bl_y_V            :out  STD_LOGIC_VECTOR(15 downto 0);
    background_delta_V    :out  STD_LOGIC_VECTOR(7 downto 0);
    min_saturation_V      :out  STD_LOGIC_VECTOR(7 downto 0);
    detected_V            :in   STD_LOGIC_VECTOR(0 downto 0);
    detected_V_ap_vld     :in   STD_LOGIC;
    area_V                :in   STD_LOGIC_VECTOR(31 downto 0);
    area_V_ap_vld         :in   STD_LOGIC;
    center_x_V            :in   STD_LOGIC_VECTOR(15 downto 0);
    center_x_V_ap_vld     :in   STD_LOGIC;
    center_y_V            :in   STD_LOGIC_VECTOR(15 downto 0);
    center_y_V_ap_vld     :in   STD_LOGIC;
    bbox_x0_V             :in   STD_LOGIC_VECTOR(15 downto 0);
    bbox_x0_V_ap_vld      :in   STD_LOGIC;
    bbox_y0_V             :in   STD_LOGIC_VECTOR(15 downto 0);
    bbox_y0_V_ap_vld      :in   STD_LOGIC;
    bbox_x1_V             :in   STD_LOGIC_VECTOR(15 downto 0);
    bbox_x1_V_ap_vld      :in   STD_LOGIC;
    bbox_y1_V             :in   STD_LOGIC_VECTOR(15 downto 0);
    bbox_y1_V_ap_vld      :in   STD_LOGIC;
    mean_hue_V            :in   STD_LOGIC_VECTOR(7 downto 0);
    mean_hue_V_ap_vld     :in   STD_LOGIC;
    color_id_V            :in   STD_LOGIC_VECTOR(3 downto 0);
    color_id_V_ap_vld     :in   STD_LOGIC;
    shape_id_V            :in   STD_LOGIC_VECTOR(3 downto 0);
    shape_id_V_ap_vld     :in   STD_LOGIC;
    class_id_V            :in   STD_LOGIC_VECTOR(7 downto 0);
    class_id_V_ap_vld     :in   STD_LOGIC;
    fill_q8_V             :in   STD_LOGIC_VECTOR(15 downto 0);
    fill_q8_V_ap_vld      :in   STD_LOGIC;
    compactness_q12_V     :in   STD_LOGIC_VECTOR(15 downto 0);
    compactness_q12_V_ap_vld :in   STD_LOGIC;
    inertia_q20_V         :in   STD_LOGIC_VECTOR(31 downto 0);
    inertia_q20_V_ap_vld  :in   STD_LOGIC;
    luma_variance_V       :in   STD_LOGIC_VECTOR(31 downto 0);
    luma_variance_V_ap_vld :in   STD_LOGIC
);
end entity vision_feature_accel_control_s_axi;

-- ------------------------Address Info-------------------
-- 0x000 : Control signals
--         bit 0  - ap_start (Read/Write/COH)
--         bit 1  - ap_done (Read/COR)
--         bit 2  - ap_idle (Read)
--         bit 3  - ap_ready (Read)
--         bit 7  - auto_restart (Read/Write)
--         others - reserved
-- 0x004 : Global Interrupt Enable Register
--         bit 0  - Global Interrupt Enable (Read/Write)
--         others - reserved
-- 0x008 : IP Interrupt Enable Register (Read/Write)
--         bit 0  - Channel 0 (ap_done)
--         bit 1  - Channel 1 (ap_ready)
--         others - reserved
-- 0x00c : IP Interrupt Status Register (Read/TOW)
--         bit 0  - Channel 0 (ap_done)
--         bit 1  - Channel 1 (ap_ready)
--         others - reserved
-- 0x010 : Data signal of roi_x0_V
--         bit 15~0 - roi_x0_V[15:0] (Read/Write)
--         others   - reserved
-- 0x014 : reserved
-- 0x018 : Data signal of roi_y0_V
--         bit 15~0 - roi_y0_V[15:0] (Read/Write)
--         others   - reserved
-- 0x01c : reserved
-- 0x020 : Data signal of roi_x1_V
--         bit 15~0 - roi_x1_V[15:0] (Read/Write)
--         others   - reserved
-- 0x024 : reserved
-- 0x028 : Data signal of roi_y1_V
--         bit 15~0 - roi_y1_V[15:0] (Read/Write)
--         others   - reserved
-- 0x02c : reserved
-- 0x030 : Data signal of detection_padding_V
--         bit 7~0 - detection_padding_V[7:0] (Read/Write)
--         others  - reserved
-- 0x034 : reserved
-- 0x038 : Data signal of roi_valid_V
--         bit 0  - roi_valid_V[0] (Read/Write)
--         others - reserved
-- 0x03c : reserved
-- 0x040 : Data signal of roi_tl_x_V
--         bit 15~0 - roi_tl_x_V[15:0] (Read/Write)
--         others   - reserved
-- 0x044 : reserved
-- 0x048 : Data signal of roi_tl_y_V
--         bit 15~0 - roi_tl_y_V[15:0] (Read/Write)
--         others   - reserved
-- 0x04c : reserved
-- 0x050 : Data signal of roi_tr_x_V
--         bit 15~0 - roi_tr_x_V[15:0] (Read/Write)
--         others   - reserved
-- 0x054 : reserved
-- 0x058 : Data signal of roi_tr_y_V
--         bit 15~0 - roi_tr_y_V[15:0] (Read/Write)
--         others   - reserved
-- 0x05c : reserved
-- 0x060 : Data signal of roi_br_x_V
--         bit 15~0 - roi_br_x_V[15:0] (Read/Write)
--         others   - reserved
-- 0x064 : reserved
-- 0x068 : Data signal of roi_br_y_V
--         bit 15~0 - roi_br_y_V[15:0] (Read/Write)
--         others   - reserved
-- 0x06c : reserved
-- 0x070 : Data signal of roi_bl_x_V
--         bit 15~0 - roi_bl_x_V[15:0] (Read/Write)
--         others   - reserved
-- 0x074 : reserved
-- 0x078 : Data signal of roi_bl_y_V
--         bit 15~0 - roi_bl_y_V[15:0] (Read/Write)
--         others   - reserved
-- 0x07c : reserved
-- 0x080 : Data signal of background_delta_V
--         bit 7~0 - background_delta_V[7:0] (Read/Write)
--         others  - reserved
-- 0x084 : reserved
-- 0x088 : Data signal of min_saturation_V
--         bit 7~0 - min_saturation_V[7:0] (Read/Write)
--         others  - reserved
-- 0x08c : reserved
-- 0x090 : Data signal of detected_V
--         bit 0  - detected_V[0] (Read)
--         others - reserved
-- 0x094 : Control signal of detected_V
--         bit 0  - detected_V_ap_vld (Read/COR)
--         others - reserved
-- 0x098 : Data signal of area_V
--         bit 31~0 - area_V[31:0] (Read)
-- 0x09c : Control signal of area_V
--         bit 0  - area_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0a0 : Data signal of center_x_V
--         bit 15~0 - center_x_V[15:0] (Read)
--         others   - reserved
-- 0x0a4 : Control signal of center_x_V
--         bit 0  - center_x_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0a8 : Data signal of center_y_V
--         bit 15~0 - center_y_V[15:0] (Read)
--         others   - reserved
-- 0x0ac : Control signal of center_y_V
--         bit 0  - center_y_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0b0 : Data signal of bbox_x0_V
--         bit 15~0 - bbox_x0_V[15:0] (Read)
--         others   - reserved
-- 0x0b4 : Control signal of bbox_x0_V
--         bit 0  - bbox_x0_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0b8 : Data signal of bbox_y0_V
--         bit 15~0 - bbox_y0_V[15:0] (Read)
--         others   - reserved
-- 0x0bc : Control signal of bbox_y0_V
--         bit 0  - bbox_y0_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0c0 : Data signal of bbox_x1_V
--         bit 15~0 - bbox_x1_V[15:0] (Read)
--         others   - reserved
-- 0x0c4 : Control signal of bbox_x1_V
--         bit 0  - bbox_x1_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0c8 : Data signal of bbox_y1_V
--         bit 15~0 - bbox_y1_V[15:0] (Read)
--         others   - reserved
-- 0x0cc : Control signal of bbox_y1_V
--         bit 0  - bbox_y1_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0d0 : Data signal of mean_hue_V
--         bit 7~0 - mean_hue_V[7:0] (Read)
--         others  - reserved
-- 0x0d4 : Control signal of mean_hue_V
--         bit 0  - mean_hue_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0d8 : Data signal of color_id_V
--         bit 3~0 - color_id_V[3:0] (Read)
--         others  - reserved
-- 0x0dc : Control signal of color_id_V
--         bit 0  - color_id_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0e0 : Data signal of shape_id_V
--         bit 3~0 - shape_id_V[3:0] (Read)
--         others  - reserved
-- 0x0e4 : Control signal of shape_id_V
--         bit 0  - shape_id_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0e8 : Data signal of class_id_V
--         bit 7~0 - class_id_V[7:0] (Read)
--         others  - reserved
-- 0x0ec : Control signal of class_id_V
--         bit 0  - class_id_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0f0 : Data signal of fill_q8_V
--         bit 15~0 - fill_q8_V[15:0] (Read)
--         others   - reserved
-- 0x0f4 : Control signal of fill_q8_V
--         bit 0  - fill_q8_V_ap_vld (Read/COR)
--         others - reserved
-- 0x0f8 : Data signal of compactness_q12_V
--         bit 15~0 - compactness_q12_V[15:0] (Read)
--         others   - reserved
-- 0x0fc : Control signal of compactness_q12_V
--         bit 0  - compactness_q12_V_ap_vld (Read/COR)
--         others - reserved
-- 0x100 : Data signal of inertia_q20_V
--         bit 31~0 - inertia_q20_V[31:0] (Read)
-- 0x104 : Control signal of inertia_q20_V
--         bit 0  - inertia_q20_V_ap_vld (Read/COR)
--         others - reserved
-- 0x108 : Data signal of luma_variance_V
--         bit 31~0 - luma_variance_V[31:0] (Read)
-- 0x10c : Control signal of luma_variance_V
--         bit 0  - luma_variance_V_ap_vld (Read/COR)
--         others - reserved
-- (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

architecture behave of vision_feature_accel_control_s_axi is
    type states is (wridle, wrdata, wrresp, wrreset, rdidle, rddata, rdreset);  -- read and write fsm states
    signal wstate  : states := wrreset;
    signal rstate  : states := rdreset;
    signal wnext, rnext: states;
    constant ADDR_AP_CTRL                    : INTEGER := 16#000#;
    constant ADDR_GIE                        : INTEGER := 16#004#;
    constant ADDR_IER                        : INTEGER := 16#008#;
    constant ADDR_ISR                        : INTEGER := 16#00c#;
    constant ADDR_ROI_X0_V_DATA_0            : INTEGER := 16#010#;
    constant ADDR_ROI_X0_V_CTRL              : INTEGER := 16#014#;
    constant ADDR_ROI_Y0_V_DATA_0            : INTEGER := 16#018#;
    constant ADDR_ROI_Y0_V_CTRL              : INTEGER := 16#01c#;
    constant ADDR_ROI_X1_V_DATA_0            : INTEGER := 16#020#;
    constant ADDR_ROI_X1_V_CTRL              : INTEGER := 16#024#;
    constant ADDR_ROI_Y1_V_DATA_0            : INTEGER := 16#028#;
    constant ADDR_ROI_Y1_V_CTRL              : INTEGER := 16#02c#;
    constant ADDR_DETECTION_PADDING_V_DATA_0 : INTEGER := 16#030#;
    constant ADDR_DETECTION_PADDING_V_CTRL   : INTEGER := 16#034#;
    constant ADDR_ROI_VALID_V_DATA_0         : INTEGER := 16#038#;
    constant ADDR_ROI_VALID_V_CTRL           : INTEGER := 16#03c#;
    constant ADDR_ROI_TL_X_V_DATA_0          : INTEGER := 16#040#;
    constant ADDR_ROI_TL_X_V_CTRL            : INTEGER := 16#044#;
    constant ADDR_ROI_TL_Y_V_DATA_0          : INTEGER := 16#048#;
    constant ADDR_ROI_TL_Y_V_CTRL            : INTEGER := 16#04c#;
    constant ADDR_ROI_TR_X_V_DATA_0          : INTEGER := 16#050#;
    constant ADDR_ROI_TR_X_V_CTRL            : INTEGER := 16#054#;
    constant ADDR_ROI_TR_Y_V_DATA_0          : INTEGER := 16#058#;
    constant ADDR_ROI_TR_Y_V_CTRL            : INTEGER := 16#05c#;
    constant ADDR_ROI_BR_X_V_DATA_0          : INTEGER := 16#060#;
    constant ADDR_ROI_BR_X_V_CTRL            : INTEGER := 16#064#;
    constant ADDR_ROI_BR_Y_V_DATA_0          : INTEGER := 16#068#;
    constant ADDR_ROI_BR_Y_V_CTRL            : INTEGER := 16#06c#;
    constant ADDR_ROI_BL_X_V_DATA_0          : INTEGER := 16#070#;
    constant ADDR_ROI_BL_X_V_CTRL            : INTEGER := 16#074#;
    constant ADDR_ROI_BL_Y_V_DATA_0          : INTEGER := 16#078#;
    constant ADDR_ROI_BL_Y_V_CTRL            : INTEGER := 16#07c#;
    constant ADDR_BACKGROUND_DELTA_V_DATA_0  : INTEGER := 16#080#;
    constant ADDR_BACKGROUND_DELTA_V_CTRL    : INTEGER := 16#084#;
    constant ADDR_MIN_SATURATION_V_DATA_0    : INTEGER := 16#088#;
    constant ADDR_MIN_SATURATION_V_CTRL      : INTEGER := 16#08c#;
    constant ADDR_DETECTED_V_DATA_0          : INTEGER := 16#090#;
    constant ADDR_DETECTED_V_CTRL            : INTEGER := 16#094#;
    constant ADDR_AREA_V_DATA_0              : INTEGER := 16#098#;
    constant ADDR_AREA_V_CTRL                : INTEGER := 16#09c#;
    constant ADDR_CENTER_X_V_DATA_0          : INTEGER := 16#0a0#;
    constant ADDR_CENTER_X_V_CTRL            : INTEGER := 16#0a4#;
    constant ADDR_CENTER_Y_V_DATA_0          : INTEGER := 16#0a8#;
    constant ADDR_CENTER_Y_V_CTRL            : INTEGER := 16#0ac#;
    constant ADDR_BBOX_X0_V_DATA_0           : INTEGER := 16#0b0#;
    constant ADDR_BBOX_X0_V_CTRL             : INTEGER := 16#0b4#;
    constant ADDR_BBOX_Y0_V_DATA_0           : INTEGER := 16#0b8#;
    constant ADDR_BBOX_Y0_V_CTRL             : INTEGER := 16#0bc#;
    constant ADDR_BBOX_X1_V_DATA_0           : INTEGER := 16#0c0#;
    constant ADDR_BBOX_X1_V_CTRL             : INTEGER := 16#0c4#;
    constant ADDR_BBOX_Y1_V_DATA_0           : INTEGER := 16#0c8#;
    constant ADDR_BBOX_Y1_V_CTRL             : INTEGER := 16#0cc#;
    constant ADDR_MEAN_HUE_V_DATA_0          : INTEGER := 16#0d0#;
    constant ADDR_MEAN_HUE_V_CTRL            : INTEGER := 16#0d4#;
    constant ADDR_COLOR_ID_V_DATA_0          : INTEGER := 16#0d8#;
    constant ADDR_COLOR_ID_V_CTRL            : INTEGER := 16#0dc#;
    constant ADDR_SHAPE_ID_V_DATA_0          : INTEGER := 16#0e0#;
    constant ADDR_SHAPE_ID_V_CTRL            : INTEGER := 16#0e4#;
    constant ADDR_CLASS_ID_V_DATA_0          : INTEGER := 16#0e8#;
    constant ADDR_CLASS_ID_V_CTRL            : INTEGER := 16#0ec#;
    constant ADDR_FILL_Q8_V_DATA_0           : INTEGER := 16#0f0#;
    constant ADDR_FILL_Q8_V_CTRL             : INTEGER := 16#0f4#;
    constant ADDR_COMPACTNESS_Q12_V_DATA_0   : INTEGER := 16#0f8#;
    constant ADDR_COMPACTNESS_Q12_V_CTRL     : INTEGER := 16#0fc#;
    constant ADDR_INERTIA_Q20_V_DATA_0       : INTEGER := 16#100#;
    constant ADDR_INERTIA_Q20_V_CTRL         : INTEGER := 16#104#;
    constant ADDR_LUMA_VARIANCE_V_DATA_0     : INTEGER := 16#108#;
    constant ADDR_LUMA_VARIANCE_V_CTRL       : INTEGER := 16#10c#;
    constant ADDR_BITS         : INTEGER := 9;

    signal waddr               : UNSIGNED(ADDR_BITS-1 downto 0);
    signal wmask               : UNSIGNED(31 downto 0);
    signal aw_hs               : STD_LOGIC;
    signal w_hs                : STD_LOGIC;
    signal rdata_data          : UNSIGNED(31 downto 0);
    signal ar_hs               : STD_LOGIC;
    signal raddr               : UNSIGNED(ADDR_BITS-1 downto 0);
    signal AWREADY_t           : STD_LOGIC;
    signal WREADY_t            : STD_LOGIC;
    signal ARREADY_t           : STD_LOGIC;
    signal RVALID_t            : STD_LOGIC;
    -- internal registers
    signal int_ap_idle         : STD_LOGIC;
    signal int_ap_ready        : STD_LOGIC;
    signal int_ap_done         : STD_LOGIC := '0';
    signal int_ap_start        : STD_LOGIC := '0';
    signal int_auto_restart    : STD_LOGIC := '0';
    signal int_gie             : STD_LOGIC := '0';
    signal int_ier             : UNSIGNED(1 downto 0) := (others => '0');
    signal int_isr             : UNSIGNED(1 downto 0) := (others => '0');
    signal int_roi_x0_V        : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_y0_V        : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_x1_V        : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_y1_V        : UNSIGNED(15 downto 0) := (others => '0');
    signal int_detection_padding_V : UNSIGNED(7 downto 0) := (others => '0');
    signal int_roi_valid_V     : UNSIGNED(0 downto 0) := (others => '0');
    signal int_roi_tl_x_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_tl_y_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_tr_x_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_tr_y_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_br_x_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_br_y_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_bl_x_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_roi_bl_y_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_background_delta_V : UNSIGNED(7 downto 0) := (others => '0');
    signal int_min_saturation_V : UNSIGNED(7 downto 0) := (others => '0');
    signal int_detected_V      : UNSIGNED(0 downto 0) := (others => '0');
    signal int_detected_V_ap_vld : STD_LOGIC;
    signal int_area_V          : UNSIGNED(31 downto 0) := (others => '0');
    signal int_area_V_ap_vld   : STD_LOGIC;
    signal int_center_x_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_center_x_V_ap_vld : STD_LOGIC;
    signal int_center_y_V      : UNSIGNED(15 downto 0) := (others => '0');
    signal int_center_y_V_ap_vld : STD_LOGIC;
    signal int_bbox_x0_V       : UNSIGNED(15 downto 0) := (others => '0');
    signal int_bbox_x0_V_ap_vld : STD_LOGIC;
    signal int_bbox_y0_V       : UNSIGNED(15 downto 0) := (others => '0');
    signal int_bbox_y0_V_ap_vld : STD_LOGIC;
    signal int_bbox_x1_V       : UNSIGNED(15 downto 0) := (others => '0');
    signal int_bbox_x1_V_ap_vld : STD_LOGIC;
    signal int_bbox_y1_V       : UNSIGNED(15 downto 0) := (others => '0');
    signal int_bbox_y1_V_ap_vld : STD_LOGIC;
    signal int_mean_hue_V      : UNSIGNED(7 downto 0) := (others => '0');
    signal int_mean_hue_V_ap_vld : STD_LOGIC;
    signal int_color_id_V      : UNSIGNED(3 downto 0) := (others => '0');
    signal int_color_id_V_ap_vld : STD_LOGIC;
    signal int_shape_id_V      : UNSIGNED(3 downto 0) := (others => '0');
    signal int_shape_id_V_ap_vld : STD_LOGIC;
    signal int_class_id_V      : UNSIGNED(7 downto 0) := (others => '0');
    signal int_class_id_V_ap_vld : STD_LOGIC;
    signal int_fill_q8_V       : UNSIGNED(15 downto 0) := (others => '0');
    signal int_fill_q8_V_ap_vld : STD_LOGIC;
    signal int_compactness_q12_V : UNSIGNED(15 downto 0) := (others => '0');
    signal int_compactness_q12_V_ap_vld : STD_LOGIC;
    signal int_inertia_q20_V   : UNSIGNED(31 downto 0) := (others => '0');
    signal int_inertia_q20_V_ap_vld : STD_LOGIC;
    signal int_luma_variance_V : UNSIGNED(31 downto 0) := (others => '0');
    signal int_luma_variance_V_ap_vld : STD_LOGIC;


begin
-- ----------------------- Instantiation------------------

-- ----------------------- AXI WRITE ---------------------
    AWREADY_t <=  '1' when wstate = wridle else '0';
    AWREADY   <=  AWREADY_t;
    WREADY_t  <=  '1' when wstate = wrdata else '0';
    WREADY    <=  WREADY_t;
    BRESP     <=  "00";  -- OKAY
    BVALID    <=  '1' when wstate = wrresp else '0';
    wmask     <=  (31 downto 24 => WSTRB(3), 23 downto 16 => WSTRB(2), 15 downto 8 => WSTRB(1), 7 downto 0 => WSTRB(0));
    aw_hs     <=  AWVALID and AWREADY_t;
    w_hs      <=  WVALID and WREADY_t;

    -- write FSM
    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                wstate <= wrreset;
            elsif (ACLK_EN = '1') then
                wstate <= wnext;
            end if;
        end if;
    end process;

    process (wstate, AWVALID, WVALID, BREADY)
    begin
        case (wstate) is
        when wridle =>
            if (AWVALID = '1') then
                wnext <= wrdata;
            else
                wnext <= wridle;
            end if;
        when wrdata =>
            if (WVALID = '1') then
                wnext <= wrresp;
            else
                wnext <= wrdata;
            end if;
        when wrresp =>
            if (BREADY = '1') then
                wnext <= wridle;
            else
                wnext <= wrresp;
            end if;
        when others =>
            wnext <= wridle;
        end case;
    end process;

    waddr_proc : process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (aw_hs = '1') then
                    waddr <= UNSIGNED(AWADDR(ADDR_BITS-1 downto 0));
                end if;
            end if;
        end if;
    end process;

-- ----------------------- AXI READ ----------------------
    ARREADY_t <= '1' when (rstate = rdidle) else '0';
    ARREADY <= ARREADY_t;
    RDATA   <= STD_LOGIC_VECTOR(rdata_data);
    RRESP   <= "00";  -- OKAY
    RVALID_t  <= '1' when (rstate = rddata) else '0';
    RVALID    <= RVALID_t;
    ar_hs   <= ARVALID and ARREADY_t;
    raddr   <= UNSIGNED(ARADDR(ADDR_BITS-1 downto 0));

    -- read FSM
    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                rstate <= rdreset;
            elsif (ACLK_EN = '1') then
                rstate <= rnext;
            end if;
        end if;
    end process;

    process (rstate, ARVALID, RREADY, RVALID_t)
    begin
        case (rstate) is
        when rdidle =>
            if (ARVALID = '1') then
                rnext <= rddata;
            else
                rnext <= rdidle;
            end if;
        when rddata =>
            if (RREADY = '1' and RVALID_t = '1') then
                rnext <= rdidle;
            else
                rnext <= rddata;
            end if;
        when others =>
            rnext <= rdidle;
        end case;
    end process;

    rdata_proc : process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (ar_hs = '1') then
                    case (TO_INTEGER(raddr)) is
                    when ADDR_AP_CTRL =>
                        rdata_data <= (7 => int_auto_restart, 3 => int_ap_ready, 2 => int_ap_idle, 1 => int_ap_done, 0 => int_ap_start, others => '0');
                    when ADDR_GIE =>
                        rdata_data <= (0 => int_gie, others => '0');
                    when ADDR_IER =>
                        rdata_data <= (1 => int_ier(1), 0 => int_ier(0), others => '0');
                    when ADDR_ISR =>
                        rdata_data <= (1 => int_isr(1), 0 => int_isr(0), others => '0');
                    when ADDR_ROI_X0_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_x0_V(15 downto 0), 32);
                    when ADDR_ROI_Y0_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_y0_V(15 downto 0), 32);
                    when ADDR_ROI_X1_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_x1_V(15 downto 0), 32);
                    when ADDR_ROI_Y1_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_y1_V(15 downto 0), 32);
                    when ADDR_DETECTION_PADDING_V_DATA_0 =>
                        rdata_data <= RESIZE(int_detection_padding_V(7 downto 0), 32);
                    when ADDR_ROI_VALID_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_valid_V(0 downto 0), 32);
                    when ADDR_ROI_TL_X_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_tl_x_V(15 downto 0), 32);
                    when ADDR_ROI_TL_Y_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_tl_y_V(15 downto 0), 32);
                    when ADDR_ROI_TR_X_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_tr_x_V(15 downto 0), 32);
                    when ADDR_ROI_TR_Y_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_tr_y_V(15 downto 0), 32);
                    when ADDR_ROI_BR_X_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_br_x_V(15 downto 0), 32);
                    when ADDR_ROI_BR_Y_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_br_y_V(15 downto 0), 32);
                    when ADDR_ROI_BL_X_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_bl_x_V(15 downto 0), 32);
                    when ADDR_ROI_BL_Y_V_DATA_0 =>
                        rdata_data <= RESIZE(int_roi_bl_y_V(15 downto 0), 32);
                    when ADDR_BACKGROUND_DELTA_V_DATA_0 =>
                        rdata_data <= RESIZE(int_background_delta_V(7 downto 0), 32);
                    when ADDR_MIN_SATURATION_V_DATA_0 =>
                        rdata_data <= RESIZE(int_min_saturation_V(7 downto 0), 32);
                    when ADDR_DETECTED_V_DATA_0 =>
                        rdata_data <= RESIZE(int_detected_V(0 downto 0), 32);
                    when ADDR_DETECTED_V_CTRL =>
                        rdata_data <= (0 => int_detected_V_ap_vld, others => '0');
                    when ADDR_AREA_V_DATA_0 =>
                        rdata_data <= RESIZE(int_area_V(31 downto 0), 32);
                    when ADDR_AREA_V_CTRL =>
                        rdata_data <= (0 => int_area_V_ap_vld, others => '0');
                    when ADDR_CENTER_X_V_DATA_0 =>
                        rdata_data <= RESIZE(int_center_x_V(15 downto 0), 32);
                    when ADDR_CENTER_X_V_CTRL =>
                        rdata_data <= (0 => int_center_x_V_ap_vld, others => '0');
                    when ADDR_CENTER_Y_V_DATA_0 =>
                        rdata_data <= RESIZE(int_center_y_V(15 downto 0), 32);
                    when ADDR_CENTER_Y_V_CTRL =>
                        rdata_data <= (0 => int_center_y_V_ap_vld, others => '0');
                    when ADDR_BBOX_X0_V_DATA_0 =>
                        rdata_data <= RESIZE(int_bbox_x0_V(15 downto 0), 32);
                    when ADDR_BBOX_X0_V_CTRL =>
                        rdata_data <= (0 => int_bbox_x0_V_ap_vld, others => '0');
                    when ADDR_BBOX_Y0_V_DATA_0 =>
                        rdata_data <= RESIZE(int_bbox_y0_V(15 downto 0), 32);
                    when ADDR_BBOX_Y0_V_CTRL =>
                        rdata_data <= (0 => int_bbox_y0_V_ap_vld, others => '0');
                    when ADDR_BBOX_X1_V_DATA_0 =>
                        rdata_data <= RESIZE(int_bbox_x1_V(15 downto 0), 32);
                    when ADDR_BBOX_X1_V_CTRL =>
                        rdata_data <= (0 => int_bbox_x1_V_ap_vld, others => '0');
                    when ADDR_BBOX_Y1_V_DATA_0 =>
                        rdata_data <= RESIZE(int_bbox_y1_V(15 downto 0), 32);
                    when ADDR_BBOX_Y1_V_CTRL =>
                        rdata_data <= (0 => int_bbox_y1_V_ap_vld, others => '0');
                    when ADDR_MEAN_HUE_V_DATA_0 =>
                        rdata_data <= RESIZE(int_mean_hue_V(7 downto 0), 32);
                    when ADDR_MEAN_HUE_V_CTRL =>
                        rdata_data <= (0 => int_mean_hue_V_ap_vld, others => '0');
                    when ADDR_COLOR_ID_V_DATA_0 =>
                        rdata_data <= RESIZE(int_color_id_V(3 downto 0), 32);
                    when ADDR_COLOR_ID_V_CTRL =>
                        rdata_data <= (0 => int_color_id_V_ap_vld, others => '0');
                    when ADDR_SHAPE_ID_V_DATA_0 =>
                        rdata_data <= RESIZE(int_shape_id_V(3 downto 0), 32);
                    when ADDR_SHAPE_ID_V_CTRL =>
                        rdata_data <= (0 => int_shape_id_V_ap_vld, others => '0');
                    when ADDR_CLASS_ID_V_DATA_0 =>
                        rdata_data <= RESIZE(int_class_id_V(7 downto 0), 32);
                    when ADDR_CLASS_ID_V_CTRL =>
                        rdata_data <= (0 => int_class_id_V_ap_vld, others => '0');
                    when ADDR_FILL_Q8_V_DATA_0 =>
                        rdata_data <= RESIZE(int_fill_q8_V(15 downto 0), 32);
                    when ADDR_FILL_Q8_V_CTRL =>
                        rdata_data <= (0 => int_fill_q8_V_ap_vld, others => '0');
                    when ADDR_COMPACTNESS_Q12_V_DATA_0 =>
                        rdata_data <= RESIZE(int_compactness_q12_V(15 downto 0), 32);
                    when ADDR_COMPACTNESS_Q12_V_CTRL =>
                        rdata_data <= (0 => int_compactness_q12_V_ap_vld, others => '0');
                    when ADDR_INERTIA_Q20_V_DATA_0 =>
                        rdata_data <= RESIZE(int_inertia_q20_V(31 downto 0), 32);
                    when ADDR_INERTIA_Q20_V_CTRL =>
                        rdata_data <= (0 => int_inertia_q20_V_ap_vld, others => '0');
                    when ADDR_LUMA_VARIANCE_V_DATA_0 =>
                        rdata_data <= RESIZE(int_luma_variance_V(31 downto 0), 32);
                    when ADDR_LUMA_VARIANCE_V_CTRL =>
                        rdata_data <= (0 => int_luma_variance_V_ap_vld, others => '0');
                    when others =>
                        rdata_data <= (others => '0');
                    end case;
                end if;
            end if;
        end if;
    end process;

-- ----------------------- Register logic ----------------
    interrupt            <= int_gie and (int_isr(0) or int_isr(1));
    ap_start             <= int_ap_start;
    roi_x0_V             <= STD_LOGIC_VECTOR(int_roi_x0_V);
    roi_y0_V             <= STD_LOGIC_VECTOR(int_roi_y0_V);
    roi_x1_V             <= STD_LOGIC_VECTOR(int_roi_x1_V);
    roi_y1_V             <= STD_LOGIC_VECTOR(int_roi_y1_V);
    detection_padding_V  <= STD_LOGIC_VECTOR(int_detection_padding_V);
    roi_valid_V          <= STD_LOGIC_VECTOR(int_roi_valid_V);
    roi_tl_x_V           <= STD_LOGIC_VECTOR(int_roi_tl_x_V);
    roi_tl_y_V           <= STD_LOGIC_VECTOR(int_roi_tl_y_V);
    roi_tr_x_V           <= STD_LOGIC_VECTOR(int_roi_tr_x_V);
    roi_tr_y_V           <= STD_LOGIC_VECTOR(int_roi_tr_y_V);
    roi_br_x_V           <= STD_LOGIC_VECTOR(int_roi_br_x_V);
    roi_br_y_V           <= STD_LOGIC_VECTOR(int_roi_br_y_V);
    roi_bl_x_V           <= STD_LOGIC_VECTOR(int_roi_bl_x_V);
    roi_bl_y_V           <= STD_LOGIC_VECTOR(int_roi_bl_y_V);
    background_delta_V   <= STD_LOGIC_VECTOR(int_background_delta_V);
    min_saturation_V     <= STD_LOGIC_VECTOR(int_min_saturation_V);

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ap_start <= '0';
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_AP_CTRL and WSTRB(0) = '1' and WDATA(0) = '1') then
                    int_ap_start <= '1';
                elsif (ap_ready = '1') then
                    int_ap_start <= int_auto_restart; -- clear on handshake/auto restart
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ap_done <= '0';
            elsif (ACLK_EN = '1') then
                if (ap_done = '1') then
                    int_ap_done <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_AP_CTRL) then
                    int_ap_done <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ap_idle <= '0';
            elsif (ACLK_EN = '1') then
                if (true) then
                    int_ap_idle <= ap_idle;
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ap_ready <= '0';
            elsif (ACLK_EN = '1') then
                if (true) then
                    int_ap_ready <= ap_ready;
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_auto_restart <= '0';
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_AP_CTRL and WSTRB(0) = '1') then
                    int_auto_restart <= WDATA(7);
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_gie <= '0';
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_GIE and WSTRB(0) = '1') then
                    int_gie <= WDATA(0);
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_ier <= "00";
            elsif (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_IER and WSTRB(0) = '1') then
                    int_ier <= UNSIGNED(WDATA(1 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_isr(0) <= '0';
            elsif (ACLK_EN = '1') then
                if (int_ier(0) = '1' and ap_done = '1') then
                    int_isr(0) <= '1';
                elsif (w_hs = '1' and waddr = ADDR_ISR and WSTRB(0) = '1') then
                    int_isr(0) <= int_isr(0) xor WDATA(0); -- toggle on write
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_isr(1) <= '0';
            elsif (ACLK_EN = '1') then
                if (int_ier(1) = '1' and ap_ready = '1') then
                    int_isr(1) <= '1';
                elsif (w_hs = '1' and waddr = ADDR_ISR and WSTRB(0) = '1') then
                    int_isr(1) <= int_isr(1) xor WDATA(1); -- toggle on write
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_X0_V_DATA_0) then
                    int_roi_x0_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_x0_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_Y0_V_DATA_0) then
                    int_roi_y0_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_y0_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_X1_V_DATA_0) then
                    int_roi_x1_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_x1_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_Y1_V_DATA_0) then
                    int_roi_y1_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_y1_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_DETECTION_PADDING_V_DATA_0) then
                    int_detection_padding_V(7 downto 0) <= (UNSIGNED(WDATA(7 downto 0)) and wmask(7 downto 0)) or ((not wmask(7 downto 0)) and int_detection_padding_V(7 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_VALID_V_DATA_0) then
                    int_roi_valid_V(0 downto 0) <= (UNSIGNED(WDATA(0 downto 0)) and wmask(0 downto 0)) or ((not wmask(0 downto 0)) and int_roi_valid_V(0 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_TL_X_V_DATA_0) then
                    int_roi_tl_x_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_tl_x_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_TL_Y_V_DATA_0) then
                    int_roi_tl_y_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_tl_y_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_TR_X_V_DATA_0) then
                    int_roi_tr_x_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_tr_x_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_TR_Y_V_DATA_0) then
                    int_roi_tr_y_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_tr_y_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_BR_X_V_DATA_0) then
                    int_roi_br_x_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_br_x_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_BR_Y_V_DATA_0) then
                    int_roi_br_y_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_br_y_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_BL_X_V_DATA_0) then
                    int_roi_bl_x_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_bl_x_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_ROI_BL_Y_V_DATA_0) then
                    int_roi_bl_y_V(15 downto 0) <= (UNSIGNED(WDATA(15 downto 0)) and wmask(15 downto 0)) or ((not wmask(15 downto 0)) and int_roi_bl_y_V(15 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_BACKGROUND_DELTA_V_DATA_0) then
                    int_background_delta_V(7 downto 0) <= (UNSIGNED(WDATA(7 downto 0)) and wmask(7 downto 0)) or ((not wmask(7 downto 0)) and int_background_delta_V(7 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ACLK_EN = '1') then
                if (w_hs = '1' and waddr = ADDR_MIN_SATURATION_V_DATA_0) then
                    int_min_saturation_V(7 downto 0) <= (UNSIGNED(WDATA(7 downto 0)) and wmask(7 downto 0)) or ((not wmask(7 downto 0)) and int_min_saturation_V(7 downto 0));
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_detected_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (detected_V_ap_vld = '1') then
                    int_detected_V <= UNSIGNED(detected_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_detected_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (detected_V_ap_vld = '1') then
                    int_detected_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_DETECTED_V_CTRL) then
                    int_detected_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_area_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (area_V_ap_vld = '1') then
                    int_area_V <= UNSIGNED(area_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_area_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (area_V_ap_vld = '1') then
                    int_area_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_AREA_V_CTRL) then
                    int_area_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_center_x_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (center_x_V_ap_vld = '1') then
                    int_center_x_V <= UNSIGNED(center_x_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_center_x_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (center_x_V_ap_vld = '1') then
                    int_center_x_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_CENTER_X_V_CTRL) then
                    int_center_x_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_center_y_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (center_y_V_ap_vld = '1') then
                    int_center_y_V <= UNSIGNED(center_y_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_center_y_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (center_y_V_ap_vld = '1') then
                    int_center_y_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_CENTER_Y_V_CTRL) then
                    int_center_y_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bbox_x0_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (bbox_x0_V_ap_vld = '1') then
                    int_bbox_x0_V <= UNSIGNED(bbox_x0_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bbox_x0_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (bbox_x0_V_ap_vld = '1') then
                    int_bbox_x0_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_BBOX_X0_V_CTRL) then
                    int_bbox_x0_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bbox_y0_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (bbox_y0_V_ap_vld = '1') then
                    int_bbox_y0_V <= UNSIGNED(bbox_y0_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bbox_y0_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (bbox_y0_V_ap_vld = '1') then
                    int_bbox_y0_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_BBOX_Y0_V_CTRL) then
                    int_bbox_y0_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bbox_x1_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (bbox_x1_V_ap_vld = '1') then
                    int_bbox_x1_V <= UNSIGNED(bbox_x1_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bbox_x1_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (bbox_x1_V_ap_vld = '1') then
                    int_bbox_x1_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_BBOX_X1_V_CTRL) then
                    int_bbox_x1_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bbox_y1_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (bbox_y1_V_ap_vld = '1') then
                    int_bbox_y1_V <= UNSIGNED(bbox_y1_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_bbox_y1_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (bbox_y1_V_ap_vld = '1') then
                    int_bbox_y1_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_BBOX_Y1_V_CTRL) then
                    int_bbox_y1_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_mean_hue_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (mean_hue_V_ap_vld = '1') then
                    int_mean_hue_V <= UNSIGNED(mean_hue_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_mean_hue_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (mean_hue_V_ap_vld = '1') then
                    int_mean_hue_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_MEAN_HUE_V_CTRL) then
                    int_mean_hue_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_color_id_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (color_id_V_ap_vld = '1') then
                    int_color_id_V <= UNSIGNED(color_id_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_color_id_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (color_id_V_ap_vld = '1') then
                    int_color_id_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_COLOR_ID_V_CTRL) then
                    int_color_id_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_shape_id_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (shape_id_V_ap_vld = '1') then
                    int_shape_id_V <= UNSIGNED(shape_id_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_shape_id_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (shape_id_V_ap_vld = '1') then
                    int_shape_id_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_SHAPE_ID_V_CTRL) then
                    int_shape_id_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_class_id_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (class_id_V_ap_vld = '1') then
                    int_class_id_V <= UNSIGNED(class_id_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_class_id_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (class_id_V_ap_vld = '1') then
                    int_class_id_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_CLASS_ID_V_CTRL) then
                    int_class_id_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_fill_q8_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (fill_q8_V_ap_vld = '1') then
                    int_fill_q8_V <= UNSIGNED(fill_q8_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_fill_q8_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (fill_q8_V_ap_vld = '1') then
                    int_fill_q8_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_FILL_Q8_V_CTRL) then
                    int_fill_q8_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_compactness_q12_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (compactness_q12_V_ap_vld = '1') then
                    int_compactness_q12_V <= UNSIGNED(compactness_q12_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_compactness_q12_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (compactness_q12_V_ap_vld = '1') then
                    int_compactness_q12_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_COMPACTNESS_Q12_V_CTRL) then
                    int_compactness_q12_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_inertia_q20_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (inertia_q20_V_ap_vld = '1') then
                    int_inertia_q20_V <= UNSIGNED(inertia_q20_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_inertia_q20_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (inertia_q20_V_ap_vld = '1') then
                    int_inertia_q20_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_INERTIA_Q20_V_CTRL) then
                    int_inertia_q20_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_luma_variance_V <= (others => '0');
            elsif (ACLK_EN = '1') then
                if (luma_variance_V_ap_vld = '1') then
                    int_luma_variance_V <= UNSIGNED(luma_variance_V); -- clear on read
                end if;
            end if;
        end if;
    end process;

    process (ACLK)
    begin
        if (ACLK'event and ACLK = '1') then
            if (ARESET = '1') then
                int_luma_variance_V_ap_vld <= '0';
            elsif (ACLK_EN = '1') then
                if (luma_variance_V_ap_vld = '1') then
                    int_luma_variance_V_ap_vld <= '1';
                elsif (ar_hs = '1' and raddr = ADDR_LUMA_VARIANCE_V_CTRL) then
                    int_luma_variance_V_ap_vld <= '0'; -- clear on read
                end if;
            end if;
        end if;
    end process;


-- ----------------------- Memory logic ------------------

end architecture behave;
