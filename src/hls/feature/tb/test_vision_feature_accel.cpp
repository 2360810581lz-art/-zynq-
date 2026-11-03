#include "vision_feature_accel.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Image {
    int width;
    int height;
    std::vector<unsigned char> rgb;
};

struct TestCase {
    const char *filename;
    bool expected_detection;
    int expected_color;
    int expected_shape;
    int expected_center_x;
    int expected_center_y;
    int background_delta;
    bool expanded_roi;
    int dynamic_roi_profile;
    bool invalid_roi;
};

static bool read_token(std::ifstream &stream, std::string &token)
{
    while (stream >> token) {
        if (!token.empty() && token[0] == '#') {
            std::string ignored;
            std::getline(stream, ignored);
            continue;
        }
        return true;
    }
    return false;
}

static bool load_ppm(const std::string &path, Image &image)
{
    std::ifstream stream(path.c_str(), std::ios::binary);
    std::string token;
    if (!stream || !read_token(stream, token) || token != "P6") return false;
    if (!read_token(stream, token)) return false;
    image.width = std::atoi(token.c_str());
    if (!read_token(stream, token)) return false;
    image.height = std::atoi(token.c_str());
    if (!read_token(stream, token) || std::atoi(token.c_str()) != 255) return false;
    stream.get();
    image.rgb.resize(image.width * image.height * 3);
    stream.read(reinterpret_cast<char *>(&image.rgb[0]), image.rgb.size());
    return stream.good() || stream.gcount() == static_cast<std::streamsize>(image.rgb.size());
}

static int run_case(const TestCase &test)
{
    Image image;
    if (!load_ppm(test.filename, image)) {
        std::cerr << "Cannot read " << test.filename << "\n";
        return 1;
    }
    if (image.width != VISION_FRAME_WIDTH || image.height != VISION_FRAME_HEIGHT) {
        std::cerr << "Unexpected dimensions in " << test.filename << "\n";
        return 1;
    }

    hls::stream<vision_axis_pixel_t> input;
    hls::stream<vision_axis_pixel_t> output;
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            int offset = (y * image.width + x) * 3;
            vision_axis_pixel_t pixel;
            pixel.data.range(23, 16) = image.rgb[offset];
            pixel.data.range(15, 8) = image.rgb[offset + 1];
            pixel.data.range(7, 0) = image.rgb[offset + 2];
            pixel.keep = 7;
            pixel.strb = 7;
            pixel.user = x == 0 && y == 0;
            pixel.last = x == image.width - 1;
            pixel.id = 0;
            pixel.dest = 0;
            input.write(pixel);
        }
    }

    ap_uint<1> detected;
    ap_uint<32> area;
    ap_uint<16> center_x, center_y, bbox_x0, bbox_y0, bbox_x1, bbox_y1;
    ap_uint<8> mean_hue, class_id;
    ap_uint<4> color_id, shape_id;
    ap_uint<16> fill_q8, compactness_q12;
    ap_uint<32> inertia_q20, luma_variance;
    bool dynamic_roi = test.dynamic_roi_profile != 0;
    bool current_green_roi = test.dynamic_roi_profile == 2;
    bool current_orange_roi = test.dynamic_roi_profile == 3;
    bool current_yellow_roi = test.dynamic_roi_profile == 4;
    bool current_red_roi = test.dynamic_roi_profile == 5;
    bool current_camera_roi = current_green_roi || current_orange_roi || current_yellow_roi || current_red_roi;
    int roi_x0 = current_camera_roi ? 420 : dynamic_roi ? 390 : test.expanded_roi ? 350 : 400;
    int roi_y0 = dynamic_roi ? 0 : test.expanded_roi ? 45 : 100;
    int roi_x1 = current_camera_roi ? 1080 : dynamic_roi ? 1040 : test.expanded_roi ? 980 : 940;
    int roi_y1 = current_camera_roi ? 680 : dynamic_roi ? 650 : test.expanded_roi ? 640 : 600;
    int detection_padding = test.expanded_roi && !dynamic_roi ? 32 : 0;
    int roi_tl_x = current_red_roi ? 438 : current_yellow_roi ? 438 : current_orange_roi ? 439 : current_green_roi ? 438 : dynamic_roi ? 416 : 367;
    int roi_tl_y = current_red_roi ? 46 : current_yellow_roi ? 48 : current_orange_roi ? 49 : current_green_roi ? 51 : dynamic_roi ? 25 : 55;
    int roi_tr_x = current_red_roi ? 1050 : current_yellow_roi ? 1041 : current_orange_roi ? 1041 : current_green_roi ? 1050 : dynamic_roi ? 1020 : 970;
    int roi_tr_y = current_red_roi ? 30 : current_yellow_roi ? 41 : current_orange_roi ? 41 : current_green_roi ? 37 : dynamic_roi ? 29 : 51;
    int roi_br_x = current_red_roi ? 1039 : current_yellow_roi ? 1040 : current_orange_roi ? 1039 : current_green_roi ? 1040 : dynamic_roi ? 1008 : 964;
    int roi_br_y = current_red_roi ? 611 : current_yellow_roi ? 616 : current_orange_roi ? 616 : current_green_roi ? 616 : dynamic_roi ? 606 : 636;
    int roi_bl_x = current_red_roi ? 465 : current_yellow_roi ? 461 : current_orange_roi ? 461 : current_green_roi ? 461 : dynamic_roi ? 426 : 383;
    int roi_bl_y = current_red_roi ? 644 : current_yellow_roi ? 640 : current_orange_roi ? 641 : current_green_roi ? 645 : dynamic_roi ? 619 : 636;
    int min_saturation = dynamic_roi ? 30 : test.expanded_roi ? 80 : 25;
    vision_feature_accel(
        input, output, roi_x0, roi_y0, roi_x1, roi_y1, detection_padding,
        !test.invalid_roi,
        roi_tl_x, roi_tl_y, roi_tr_x, roi_tr_y,
        roi_br_x, roi_br_y, roi_bl_x, roi_bl_y,
        test.background_delta, min_saturation,
        detected, area, center_x, center_y,
        bbox_x0, bbox_y0, bbox_x1, bbox_y1,
        mean_hue, color_id, shape_id, class_id,
        fill_q8, compactness_q12, inertia_q20, luma_variance);

    int pixel_count = 0;
    bool passthrough_ok = true;
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            vision_axis_pixel_t pixel = output.read();
            int offset = (y * image.width + x) * 3;
            ap_uint<24> expected =
                (ap_uint<24>(image.rgb[offset]) << 16) |
                (ap_uint<24>(image.rgb[offset + 1]) << 8) |
                image.rgb[offset + 2];
            if (pixel.data != expected || pixel.user != (x == 0 && y == 0) ||
                pixel.last != (x == image.width - 1)) {
                passthrough_ok = false;
            }
            ++pixel_count;
        }
    }

    std::cout << test.filename
              << " detected=" << detected
              << " area=" << area
              << " center=(" << center_x << "," << center_y << ")"
              << " bbox=(" << bbox_x0 << "," << bbox_y0 << ")-("
              << bbox_x1 << "," << bbox_y1 << ")"
              << " hue=" << mean_hue
              << " color=" << color_id
              << " shape=" << shape_id
              << " class=" << class_id
              << " fill_q8=" << fill_q8
              << " compactness_q12=" << compactness_q12
              << " inertia_q20=" << inertia_q20
              << " variance=" << luma_variance << "\n";

    int delta_x = int(center_x) - test.expected_center_x;
    int delta_y = int(center_y) - test.expected_center_y;
    bool center_ok = test.expected_center_x < 0 ||
        (delta_x >= -35 && delta_x <= 35 && delta_y >= -35 && delta_y <= 35);
    bool bbox_ok = dynamic_roi
        ? bbox_x0 >= roi_x0 && bbox_y0 >= roi_y0 &&
            bbox_x1 <= roi_x1 && bbox_y1 <= roi_y1
        : test.expanded_roi
        ? bbox_x0 >= 350 && bbox_y0 >= 45 && bbox_x1 <= 980 && bbox_y1 <= 640
        : bbox_x0 >= 400 && bbox_y0 >= 100 && bbox_x1 <= 940 && bbox_y1 <= 600;
    bool features_ok = test.expected_detection
        ? detected && area >= 1000 && int(color_id) == test.expected_color &&
            int(shape_id) == test.expected_shape &&
            center_ok && bbox_ok &&
            bbox_x1 - bbox_x0 <= 400 && bbox_y1 - bbox_y0 <= 400 &&
            class_id == (test.expected_color != 0 && test.expected_shape != 0
                ? ((test.expected_color << 4) | test.expected_shape) : 0)
        : !detected && color_id == 0 && shape_id == 0 && class_id == 0;
    if (!features_ok || pixel_count != VISION_FRAME_WIDTH * VISION_FRAME_HEIGHT ||
        !passthrough_ok) {
        std::cerr << "Feature test failed for " << test.filename << "\n";
        return 1;
    }
    return 0;
}

int main()
{
    const TestCase tests[] = {
        {"background.ppm", false, 0, 0, 0, 0, 22},
        {"blue_cube_center_01.ppm", true, 5, 4, 674, 342, 0},
        {"red_cylinder_center_01.ppm", true, 1, 1, 652, 339, 0},
        {"red_cylinder_upper_right_01.ppm", true, 1, 1, 876, 149, 0, true},
        {"orange_cylinder_center_01.ppm", true, 2, 1, 627, 382, 0},
        {"yellow_pentagonal_prism_center_01.ppm", true, 3, 3, 645, 318, 0},
        {"green_triangular_prism_center_01.ppm", true, 4, 2, 644, 342, 0},
        {"blue_cube_atomic_01.ppm", true, 5, 4, -1, -1, 22},
        {"red_cylinder_atomic_01.ppm", true, 1, 1, -1, -1, 22},
        {"orange_cylinder_atomic_01.ppm", true, 2, 1, -1, -1, 22},
        {"yellow_pentagonal_prism_atomic_01.ppm", true, 3, 3, -1, -1, 22},
        {"green_triangular_prism_atomic_01.ppm", true, 4, 2, -1, -1, 22},
        {"green_triangular_prism_atomic_02.ppm", true, 4, 2, -1, -1, 22},
        {"blue_cube_lower_right_dynamic_roi_01.ppm", true, 5, 4, -1, -1, 0, true, true},
        {"green_triangular_prism_upper_left_dynamic_roi_01.ppm", true, 4, 2, -1, -1, 0, true, 2},
        {"orange_cylinder_right_dynamic_roi_01.ppm", true, 2, 1, -1, -1, 0, true, 3},
        {"yellow_pentagonal_prism_bottom_dynamic_roi_01.ppm", true, 3, 3, -1, -1, 0, true, 4},
        {"red_cylinder_lower_left_dynamic_roi_01.ppm", true, 1, 1, -1, -1, 0, true, 5},
        {"blue_cube_center_01.ppm", false, 0, 0, 0, 0, 0, false, false, true}
    };
    int failures = 0;
    for (unsigned int index = 0; index < sizeof(tests) / sizeof(tests[0]); ++index) {
        failures += run_case(tests[index]);
    }
    if (failures) {
        std::cerr << failures << " HLS feature tests failed\n";
        return 1;
    }
    std::cout << "All HLS feature tests passed\n";
    return 0;
}
