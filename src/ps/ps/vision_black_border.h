#ifndef VISION_BLACK_BORDER_H
#define VISION_BLACK_BORDER_H
#include "vision_frame_view.h"

typedef struct {
    uint32_t dark_threshold, close_kernel, interior_threshold;
    uint32_t interior_close_kernel, interior_dilation_kernel;
    double corner_radius, bottom_inset, min_confidence;
} VisionBlackBorderSettings;

typedef struct {
    double corners[8]; /* TL, TR, BR, BL in source pixels */
    double confidence, area_fraction;
    double support[3], residual[3]; /* left, right, bottom */
    uint32_t bbox[4];
    uint64_t generation;
} VisionBlackBorderResult;

void VisionBlackBorder_Defaults(VisionBlackBorderSettings *settings);
uint32_t VisionBlackBorder_ScratchBytes(uint32_t width, uint32_t height);

/* Serialized CPU0 operation on an immutable owned BGR frame. Scratch must be
 * 8-byte aligned and disjoint from frame, settings and result. No allocation,
 * cache or MMIO. Every failure clears result, including its generation.
 * White-component extrema seed local Shi-Tomasi corners; this deliberately
 * omits Python's contour approximation/global feature selection. Historical
 * recognition regression is required before accepting changed calibration. */
int VisionBlackBorder_Detect(const VisionFrameView *frame,
    const VisionBlackBorderSettings *settings, void *scratch,
    uint32_t scratch_bytes, VisionBlackBorderResult *result);
#endif
