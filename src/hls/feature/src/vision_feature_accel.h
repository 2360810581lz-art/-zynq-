#ifndef VISION_FEATURE_ACCEL_H
#define VISION_FEATURE_ACCEL_H

#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

static const int VISION_FRAME_WIDTH = 1280;
static const int VISION_FRAME_HEIGHT = 720;

typedef ap_axiu<24, 1, 1, 1> vision_axis_pixel_t;

void vision_feature_accel(
    hls::stream<vision_axis_pixel_t> &video_in,
    hls::stream<vision_axis_pixel_t> &video_out,
    ap_uint<16> roi_x0,
    ap_uint<16> roi_y0,
    ap_uint<16> roi_x1,
    ap_uint<16> roi_y1,
    ap_uint<8> detection_padding,
    ap_uint<1> roi_valid,
    ap_uint<16> roi_tl_x,
    ap_uint<16> roi_tl_y,
    ap_uint<16> roi_tr_x,
    ap_uint<16> roi_tr_y,
    ap_uint<16> roi_br_x,
    ap_uint<16> roi_br_y,
    ap_uint<16> roi_bl_x,
    ap_uint<16> roi_bl_y,
    ap_uint<8> background_delta,
    ap_uint<8> min_saturation,
    ap_uint<1> &detected,
    ap_uint<32> &area,
    ap_uint<16> &center_x,
    ap_uint<16> &center_y,
    ap_uint<16> &bbox_x0,
    ap_uint<16> &bbox_y0,
    ap_uint<16> &bbox_x1,
    ap_uint<16> &bbox_y1,
    ap_uint<8> &mean_hue,
    ap_uint<4> &color_id,
    ap_uint<4> &shape_id,
    ap_uint<8> &class_id,
    ap_uint<16> &fill_q8,
    ap_uint<16> &compactness_q12,
    ap_uint<32> &inertia_q20,
    ap_uint<32> &luma_variance);

#endif
