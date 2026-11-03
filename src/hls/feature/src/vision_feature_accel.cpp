#include "vision_feature_accel.h"
#include "background_lut.h"

namespace {

ap_uint<8> maximum3(ap_uint<8> a, ap_uint<8> b, ap_uint<8> c)
{
    ap_uint<8> maximum = a > b ? a : b;
    return maximum > c ? maximum : c;
}

ap_uint<8> minimum3(ap_uint<8> a, ap_uint<8> b, ap_uint<8> c)
{
    ap_uint<8> minimum = a < b ? a : b;
    return minimum < c ? minimum : c;
}

struct EdgeStepper {
    ap_int<18> x_step;
    ap_int<18> y_step;
    ap_int<37> initial_value;
};

EdgeStepper make_edge_stepper(
    ap_uint<16> ax,
    ap_uint<16> ay,
    ap_uint<16> bx,
    ap_uint<16> by,
    ap_uint<8> detection_padding)
{
    ap_int<18> edge_x = ap_int<18>(bx) - ap_int<18>(ax);
    ap_int<18> edge_y = ap_int<18>(by) - ap_int<18>(ay);
    ap_uint<18> absolute_edge_x;
    ap_uint<18> absolute_edge_y;
    if (edge_x < 0) {
        absolute_edge_x = ap_uint<18>(-ap_int<19>(edge_x));
    } else {
        absolute_edge_x = ap_uint<18>(edge_x);
    }
    if (edge_y < 0) {
        absolute_edge_y = ap_uint<18>(-ap_int<19>(edge_y));
    } else {
        absolute_edge_y = ap_uint<18>(edge_y);
    }
    ap_uint<19> edge_scale = absolute_edge_x + absolute_edge_y;
    ap_int<37> origin_cross = edge_y * ap_int<18>(ax) -
        edge_x * ap_int<18>(ay);
    ap_uint<27> padding_offset =
        ap_uint<27>(detection_padding) * edge_scale;

    EdgeStepper stepper;
    stepper.x_step = -edge_y;
    stepper.y_step = edge_x;
    stepper.initial_value = origin_cross + ap_int<37>(padding_offset);
    return stepper;
}

ap_uint<8> rgb_to_hue(ap_uint<8> red, ap_uint<8> green, ap_uint<8> blue)
{
    ap_uint<8> maximum = maximum3(red, green, blue);
    ap_uint<8> minimum = minimum3(red, green, blue);
    ap_uint<8> delta = maximum - minimum;
    if (delta == 0) {
        return 0;
    }

    ap_int<18> hue;
    if (maximum == red) {
        hue = 30 * (ap_int<10>(green) - ap_int<10>(blue)) / delta;
        if (hue < 0) {
            hue += 180;
        }
    } else if (maximum == green) {
        hue = 60 + 30 * (ap_int<10>(blue) - ap_int<10>(red)) / delta;
    } else {
        hue = 120 + 30 * (ap_int<10>(red) - ap_int<10>(green)) / delta;
    }
    if (hue < 0) {
        hue += 180;
    }
    if (hue >= 180) {
        hue -= 180;
    }
    return ap_uint<8>(hue);
}

ap_uint<8> circular_hue_distance(ap_uint<8> hue, ap_uint<8> center)
{
    ap_uint<8> distance = hue > center ? hue - center : center - hue;
    return distance > 90 ? ap_uint<8>(180 - distance) : distance;
}

ap_uint<8> rgb_to_saturation(ap_uint<8> red, ap_uint<8> green, ap_uint<8> blue)
{
    ap_uint<8> maximum = maximum3(red, green, blue);
    ap_uint<8> minimum = minimum3(red, green, blue);
    return maximum == 0
        ? ap_uint<8>(0)
        : ap_uint<8>((ap_uint<16>(maximum - minimum) * 255) / maximum);
}

ap_uint<4> classify_color(ap_uint<8> hue, ap_uint<8> saturation)
{
#pragma HLS INLINE off
#pragma HLS PIPELINE II=1
#pragma HLS LATENCY min=6
    const ap_uint<8> centers[6] = {0, 7, 15, 28, 60, 110};
    const ap_uint<8> tolerances[6] = {0, 8, 7, 12, 18, 20};
    ap_uint<8> best_distance = 180;
    ap_uint<4> best_id = 0;

    for (int id = 1; id <= 5; ++id) {
#pragma HLS UNROLL
        ap_uint<8> distance = circular_hue_distance(hue, centers[id]);
        // Prefer the later calibrated band at an exact midpoint. This makes
        // hue 11 orange while preserving the observed red range at hue 8-9.
        if (distance <= tolerances[id] && distance <= best_distance) {
            best_distance = distance;
            best_id = id;
        }
    }
    ap_uint<8> wrap_distance = circular_hue_distance(hue, 179);
    if (wrap_distance <= tolerances[1] && wrap_distance < best_distance) {
        best_id = 1;
    }
    // Red and orange meet at the integer hue midpoint (11). Their calibrated
    // saturation bands are separated, so retain high-saturation red samples.
    if (best_id == 2 && hue == 11 && saturation >= 150) {
        best_id = 1;
    }
    return best_id;
}

bool background_different(ap_uint<24> pixel, ap_uint<24> background, ap_uint<8> threshold)
{
#pragma HLS INLINE off
#pragma HLS PIPELINE II=1
#pragma HLS LATENCY min=2
    bool different = threshold == 0;
    for (int channel = 0; channel < 3; ++channel) {
#pragma HLS UNROLL
        ap_uint<8> a = pixel.range(channel * 8 + 7, channel * 8);
        ap_uint<8> b = background.range(channel * 8 + 7, channel * 8);
        ap_uint<8> delta = a > b ? ap_uint<8>(a - b) : ap_uint<8>(b - a);
        different |= delta >= threshold;
    }
    return different;
}

// Conservative fixed-point classifier for the calibrated object presentations.
// Ambiguous feature combinations intentionally remain unknown instead of
// creating a false route command.
ap_uint<4> classify_shape(
    ap_uint<16> fill_q8,
    ap_uint<32> inertia_q20,
    ap_uint<4> color_id)
{
    if (fill_q8 <= 175 && inertia_q20 >= 220000) {
        return 2;  // triangular_prism
    }
    if (fill_q8 >= 230) {
        return 4;  // cube
    }
    // The active yellow pentagonal samples overlap cylinders in the two
    // silhouette-only features. Keep the calibrated color cue, but do not
    // cap inertia: perspective at the ROI edge changes it substantially.
    if (color_id == 3 && fill_q8 >= 170 && fill_q8 <= 220 &&
        inertia_q20 >= 190000) {
        return 3;  // pentagonal_prism
    }
    if (fill_q8 >= 185) {
        return 1;  // cylinder
    }
    return 0;
}

}  // namespace

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
    ap_uint<32> &luma_variance)
{
#pragma HLS INTERFACE axis port=video_in
#pragma HLS INTERFACE axis port=video_out
#pragma HLS INTERFACE s_axilite port=roi_x0 bundle=control
#pragma HLS INTERFACE s_axilite port=roi_y0 bundle=control
#pragma HLS INTERFACE s_axilite port=roi_x1 bundle=control
#pragma HLS INTERFACE s_axilite port=roi_y1 bundle=control
#pragma HLS INTERFACE s_axilite port=detection_padding bundle=control
#pragma HLS INTERFACE s_axilite port=roi_valid bundle=control
#pragma HLS INTERFACE s_axilite port=roi_tl_x bundle=control
#pragma HLS INTERFACE s_axilite port=roi_tl_y bundle=control
#pragma HLS INTERFACE s_axilite port=roi_tr_x bundle=control
#pragma HLS INTERFACE s_axilite port=roi_tr_y bundle=control
#pragma HLS INTERFACE s_axilite port=roi_br_x bundle=control
#pragma HLS INTERFACE s_axilite port=roi_br_y bundle=control
#pragma HLS INTERFACE s_axilite port=roi_bl_x bundle=control
#pragma HLS INTERFACE s_axilite port=roi_bl_y bundle=control
#pragma HLS INTERFACE s_axilite port=background_delta bundle=control
#pragma HLS INTERFACE s_axilite port=min_saturation bundle=control
#pragma HLS INTERFACE s_axilite port=detected bundle=control
#pragma HLS INTERFACE s_axilite port=area bundle=control
#pragma HLS INTERFACE s_axilite port=center_x bundle=control
#pragma HLS INTERFACE s_axilite port=center_y bundle=control
#pragma HLS INTERFACE s_axilite port=bbox_x0 bundle=control
#pragma HLS INTERFACE s_axilite port=bbox_y0 bundle=control
#pragma HLS INTERFACE s_axilite port=bbox_x1 bundle=control
#pragma HLS INTERFACE s_axilite port=bbox_y1 bundle=control
#pragma HLS INTERFACE s_axilite port=mean_hue bundle=control
#pragma HLS INTERFACE s_axilite port=color_id bundle=control
#pragma HLS INTERFACE s_axilite port=shape_id bundle=control
#pragma HLS INTERFACE s_axilite port=class_id bundle=control
#pragma HLS INTERFACE s_axilite port=fill_q8 bundle=control
#pragma HLS INTERFACE s_axilite port=compactness_q12 bundle=control
#pragma HLS INTERFACE s_axilite port=inertia_q20 bundle=control
#pragma HLS INTERFACE s_axilite port=luma_variance bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    ap_uint<1> previous_line[VISION_FRAME_WIDTH];
#pragma HLS RESOURCE variable=previous_line core=RAM_S2P_BRAM
    ap_uint<10> column_occupancy[VISION_FRAME_WIDTH];
#pragma HLS RESOURCE variable=column_occupancy core=RAM_S2P_BRAM
#pragma HLS RESOURCE variable=VISION_BACKGROUND_LUT core=ROM_1P_BRAM

    ap_uint<32> area_accumulator = 0;
    ap_uint<48> x_accumulator = 0;
    ap_uint<48> y_accumulator = 0;
    ap_uint<56> x2_accumulator = 0;
    ap_uint<56> y2_accumulator = 0;
    ap_uint<40> red_accumulator = 0;
    ap_uint<40> green_accumulator = 0;
    ap_uint<40> blue_accumulator = 0;
    ap_uint<40> luma_accumulator = 0;
    ap_uint<56> luma2_accumulator = 0;
    ap_uint<32> perimeter_accumulator = 0;
    ap_uint<16> row_run_start = 0;
    ap_uint<16> row_run_length = 0;
    ap_uint<16> best_row_start = 0;
    ap_uint<16> best_row_length = 0;
    const ap_uint<10> minimum_projection = 16;

    // Corners arrive clockwise in TL,TR,BR,BL order. Each edge is represented
    // as A*x+B*y+C. Products are evaluated once per frame; the pixel loop then
    // advances the four signed edge values with additions only.
    EdgeStepper top_edge = make_edge_stepper(
        roi_tl_x, roi_tl_y, roi_tr_x, roi_tr_y, detection_padding);
    EdgeStepper right_edge = make_edge_stepper(
        roi_tr_x, roi_tr_y, roi_br_x, roi_br_y, detection_padding);
    EdgeStepper bottom_edge = make_edge_stepper(
        roi_br_x, roi_br_y, roi_bl_x, roi_bl_y, detection_padding);
    EdgeStepper left_edge = make_edge_stepper(
        roi_bl_x, roi_bl_y, roi_tl_x, roi_tl_y, detection_padding);
    ap_int<37> top_row_value = top_edge.initial_value;
    ap_int<37> right_row_value = right_edge.initial_value;
    ap_int<37> bottom_row_value = bottom_edge.initial_value;
    ap_int<37> left_row_value = left_edge.initial_value;

    for (int y = 0; y < VISION_FRAME_HEIGHT; ++y) {
        ap_uint<1> left_mask = 0;
        ap_uint<16> row_occupancy = 0;
        int background_row = (y >> VISION_BACKGROUND_BLOCK_SHIFT) * VISION_BACKGROUND_WIDTH;
        ap_int<37> top_edge_value = top_row_value;
        ap_int<37> right_edge_value = right_row_value;
        ap_int<37> bottom_edge_value = bottom_row_value;
        ap_int<37> left_edge_value = left_row_value;
        for (int x = 0; x < VISION_FRAME_WIDTH; ++x) {
#pragma HLS PIPELINE II=1
            vision_axis_pixel_t pixel = video_in.read();
            video_out.write(pixel);

            ap_uint<8> blue = pixel.data.range(7, 0);
            ap_uint<8> green = pixel.data.range(15, 8);
            ap_uint<8> red = pixel.data.range(23, 16);
            ap_uint<8> maximum = maximum3(red, green, blue);
            ap_uint<8> minimum = minimum3(red, green, blue);
            ap_uint<8> chroma = maximum - minimum;
            ap_uint<24> background = VISION_BACKGROUND_LUT[
                background_row + (x >> VISION_BACKGROUND_BLOCK_SHIFT)];
            ap_uint<16> saturation_left = ap_uint<16>(chroma) * 255;
            ap_uint<16> saturation_right = ap_uint<16>(maximum) * min_saturation;
            bool inside_quad = top_edge_value >= 0 && right_edge_value >= 0 &&
                bottom_edge_value >= 0 && left_edge_value >= 0;
            bool in_roi = roi_valid && x >= roi_x0 && x <= roi_x1 &&
                y >= roi_y0 && y <= roi_y1 && inside_quad;
            // A zero background_delta explicitly bypasses the static background
            // comparison. This is useful when calibration frames were not
            // captured atomically with the object frame.
            bool background_match = background_different(pixel.data, background, background_delta);
            ap_uint<1> foreground = in_roi && background_match &&
                maximum > 0 && saturation_left >= saturation_right;
            ap_uint<1> above_mask = y == 0 ? ap_uint<1>(0) : previous_line[x];

            ap_uint<3> boundary_count = ap_uint<1>(foreground != left_mask) +
                ap_uint<1>(foreground != above_mask) +
                ap_uint<1>(x == VISION_FRAME_WIDTH - 1 && foreground) +
                ap_uint<1>(y == VISION_FRAME_HEIGHT - 1 && foreground);
            perimeter_accumulator += boundary_count;
            previous_line[x] = foreground;
            left_mask = foreground;
            ap_uint<10> previous_occupancy = y == 0 ? ap_uint<10>(0) : column_occupancy[x];
            column_occupancy[x] = previous_occupancy + foreground;

            if (foreground) {
                ++area_accumulator;
                ++row_occupancy;
                x_accumulator += x;
                y_accumulator += y;
                x2_accumulator += x * x;
                y2_accumulator += y * y;
                red_accumulator += red;
                green_accumulator += green;
                blue_accumulator += blue;
                ap_uint<16> luma = (77 * red + 150 * green + 29 * blue) >> 8;
                luma_accumulator += luma;
                luma2_accumulator += luma * luma;
            }
            top_edge_value += top_edge.x_step;
            right_edge_value += right_edge.x_step;
            bottom_edge_value += bottom_edge.x_step;
            left_edge_value += left_edge.x_step;
        }
        top_row_value += top_edge.y_step;
        right_row_value += right_edge.y_step;
        bottom_row_value += bottom_edge.y_step;
        left_row_value += left_edge.y_step;
        if (row_occupancy >= minimum_projection) {
            if (row_run_length == 0) row_run_start = y;
            ++row_run_length;
        } else {
            if (row_run_length > best_row_length) {
                best_row_start = row_run_start;
                best_row_length = row_run_length;
            }
            row_run_length = 0;
        }
    }
    if (row_run_length > best_row_length) {
        best_row_start = row_run_start;
        best_row_length = row_run_length;
    }

    ap_uint<16> column_run_start = 0;
    ap_uint<16> column_run_length = 0;
    ap_uint<16> best_column_start = 0;
    ap_uint<16> best_column_length = 0;
    for (int x = 0; x < VISION_FRAME_WIDTH; ++x) {
#pragma HLS PIPELINE II=1
        if (column_occupancy[x] >= minimum_projection) {
            if (column_run_length == 0) column_run_start = x;
            ++column_run_length;
        } else {
            if (column_run_length > best_column_length) {
                best_column_start = column_run_start;
                best_column_length = column_run_length;
            }
            column_run_length = 0;
        }
    }
    if (column_run_length > best_column_length) {
        best_column_start = column_run_start;
        best_column_length = column_run_length;
    }

    detected = area_accumulator >= 300 && best_column_length > 0 && best_row_length > 0;
    area = area_accumulator;
    shape_id = 0;
    class_id = 0;
    if (!detected) {
        center_x = 0;
        center_y = 0;
        bbox_x0 = 0;
        bbox_y0 = 0;
        bbox_x1 = 0;
        bbox_y1 = 0;
        mean_hue = 0;
        color_id = 0;
        fill_q8 = 0;
        compactness_q12 = 0;
        inertia_q20 = 0;
        luma_variance = 0;
        return;
    }

    bbox_x0 = best_column_start;
    bbox_y0 = best_row_start;
    bbox_x1 = best_column_start + best_column_length - 1;
    bbox_y1 = best_row_start + best_row_length - 1;
    // Use the center of the longest projection runs for positioning. The raw
    // foreground centroid is vulnerable to sparse black-line/background noise
    // near the calibrated edges, while those pixels are rejected by the
    // projection runs used to form this main-object box.
    center_x = best_column_start + (best_column_length >> 1);
    center_y = best_row_start + (best_row_length >> 1);
    ap_uint<16> box_width = best_column_length;
    ap_uint<16> box_height = best_row_length;
    ap_uint<32> box_area = box_width * box_height;
    fill_q8 = (ap_uint<48>(area_accumulator) << 8) / box_area;

    ap_uint<64> perimeter_squared = ap_uint<64>(perimeter_accumulator) * perimeter_accumulator;
    compactness_q12 = perimeter_squared == 0
        ? ap_uint<16>(0)
        : ap_uint<16>((ap_uint<64>(area_accumulator) << 12) / perimeter_squared);

    ap_uint<64> mean_x_term = (x_accumulator * x_accumulator) / area_accumulator;
    ap_uint<64> mean_y_term = (y_accumulator * y_accumulator) / area_accumulator;
    ap_uint<64> mu20 = x2_accumulator > mean_x_term
        ? ap_uint<64>(x2_accumulator - mean_x_term)
        : ap_uint<64>(0);
    ap_uint<64> mu02 = y2_accumulator > mean_y_term
        ? ap_uint<64>(y2_accumulator - mean_y_term)
        : ap_uint<64>(0);
    ap_uint<64> area_squared = ap_uint<64>(area_accumulator) * area_accumulator;
    inertia_q20 = area_squared == 0
        ? ap_uint<32>(0)
        : ap_uint<32>(((mu20 + mu02) << 20) / area_squared);

    ap_uint<8> average_red = red_accumulator / area_accumulator;
    ap_uint<8> average_green = green_accumulator / area_accumulator;
    ap_uint<8> average_blue = blue_accumulator / area_accumulator;
    mean_hue = rgb_to_hue(average_red, average_green, average_blue);
    ap_uint<8> average_saturation = rgb_to_saturation(
        average_red, average_green, average_blue);
    color_id = classify_color(mean_hue, average_saturation);

    ap_uint<32> average_luma = luma_accumulator / area_accumulator;
    ap_uint<64> mean_luma_squared = luma2_accumulator / area_accumulator;
    luma_variance = mean_luma_squared > average_luma * average_luma
        ? ap_uint<32>(mean_luma_squared - average_luma * average_luma)
        : ap_uint<32>(0);
    shape_id = classify_shape(fill_q8, inertia_q20, color_id);
    class_id = color_id != 0 && shape_id != 0
        ? ap_uint<8>((ap_uint<8>(color_id) << 4) | shape_id)
        : ap_uint<8>(0);
}
