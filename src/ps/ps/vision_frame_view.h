#ifndef VISION_FRAME_VIEW_H
#define VISION_FRAME_VIEW_H
#include <stdint.h>

typedef struct {
    const uint8_t *bgr;
    uint32_t width,height,stride,bytes;
    uint64_t generation;
} VisionFrameView;
#endif
