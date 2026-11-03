#ifndef LIVE_OBJECT_COLOR_H
#define LIVE_OBJECT_COLOR_H
#include <ap_axi_sdata.h>
#include <hls_stream.h>
typedef ap_axiu<32,1,1,1> color_pixel;
typedef ap_axiu<64,1,1,1> color_members;
typedef ap_axiu<64,1,1,1> color_result;
/* Membership: final parts masks, slot order, followed by zero LAST terminal.
 * Pixels: BGR[23:0], accepted part ID[30:24], bit31 zero; USER first,
 * LAST per row. These must be immutable outputs from the SAME frame.
 * Result: color[2:0] (0 UNK,1 red,2 orange,3 yellow,4 green,5 blue),
 * shape[5:3]=0 UNK, vote fraction[21:6], eligible[39:22], sampled[57:40].
 * Reserved high bits zero. One result per slot plus zero LAST terminal.
 * Status:1 config;2 membership/framing/overlap;4 pixels/framing;
 * 8 empty object. Any error suppresses ALL results; terminal always emitted.
 * Scalar invalid consumes no inputs; other errors drain bounded inputs.
 * Fixed provisional color rules, NOT calibrated recognition accuracy.
 * Parent must qualify final status, frame token, upstream faults and ROI.
 */
void live_object_color(hls::stream<color_members>& membership,
    hls::stream<color_pixel>& pixels,hls::stream<color_result>& results,
    ap_uint<16> width,ap_uint<16> height,ap_uint<8> part_count,
    ap_uint<4> object_count,ap_uint<4>& status);
#endif
