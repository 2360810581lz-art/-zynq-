#ifndef LIVE_DETECTOR_H
#define LIVE_DETECTOR_H
#include "vision_frame_view.h"

/* Dynamic color-foreground boxes with a compact semantic tag.
 * Serialized calls use static scratch; input must remain immutable. */
typedef struct { uint32_t bbox[4], center[2], match_error; } LiveObject;

/*
 * The original result ABI has no spare word.  The old match_error word was
 * always zero, so it is now a versioned color/shape tag while retaining the
 * same 268-byte LiveResult layout.  Bits 0..3 and 4..7 are the configured
 * color/shape IDs, bit 8 says both IDs are valid, and bits 16..23 carry the
 * conservative minimum confidence in Q0.8.  Bits 24..31 are reserved.
 */
#define LIVE_TAG_COLOR_SHIFT 0u
#define LIVE_TAG_SHAPE_SHIFT 4u
#define LIVE_TAG_VALID_SHIFT 8u
#define LIVE_TAG_CONF_SHIFT 16u
#define LIVE_TAG_NIBBLE_MASK 0x0fu
#define LIVE_TAG_CONF_MASK 0xffu
#define LIVE_TAG_PACK(color, shape, valid, confidence) \
    ((((uint32_t)(color) & LIVE_TAG_NIBBLE_MASK) << LIVE_TAG_COLOR_SHIFT) | \
     (((uint32_t)(shape) & LIVE_TAG_NIBBLE_MASK) << LIVE_TAG_SHAPE_SHIFT) | \
     (((uint32_t)(valid) & 1u) << LIVE_TAG_VALID_SHIFT) | \
     (((uint32_t)(confidence) & LIVE_TAG_CONF_MASK) << LIVE_TAG_CONF_SHIFT))
#define LIVE_TAG_COLOR(tag) (((tag) >> LIVE_TAG_COLOR_SHIFT) & LIVE_TAG_NIBBLE_MASK)
#define LIVE_TAG_SHAPE(tag) (((tag) >> LIVE_TAG_SHAPE_SHIFT) & LIVE_TAG_NIBBLE_MASK)
#define LIVE_TAG_VALID(tag) (((tag) >> LIVE_TAG_VALID_SHIFT) & 1u)
#define LIVE_TAG_CONFIDENCE(tag) (((tag) >> LIVE_TAG_CONF_SHIFT) & LIVE_TAG_CONF_MASK)
#define LIVE_TAG_CLASS(tag) ((LIVE_TAG_COLOR(tag) << 4) | LIVE_TAG_SHAPE(tag))
typedef struct {
    uint32_t valid, count, roi[8];
    LiveObject objects[8];
    uint32_t overflow; /* additional candidates beyond the eight-box capacity */
} LiveResult;
/* Kept by the caller and snapshotted before detection for independent replay.
 * mode: 1=located, 2=edge-verified reuse, 3=bounded occlusion hold. */
typedef struct {
    uint32_t locked, misses, mode, reserved;
    uint32_t roi[8];
    uint64_t generation;
    uint64_t now_ms, last_good_ms; /* caller supplies a monotonic clock */
} LiveRoiState;
/* Draw-only jitter suppression with at most four pixels of edge lag.
 * Missing objects and center motion beyond four pixels follow raw immediately.
 * Zero-initialize and serialize. */
typedef struct { LiveResult shown; uint64_t stamp_ms; } LiveBoxState;
void LiveStabilize(const LiveResult *raw, LiveResult *shown,
                   LiveBoxState *state, uint64_t now_ms);
int LiveDetect(const VisionFrameView *frame, LiveResult *result);
int LiveDetectState(const VisionFrameView *frame, LiveResult *result, LiveRoiState *state);
#ifdef LIVE_PL_HANDOFF
/* Candidate-only entry points. The serialized coordinator must pin and
 * invalidate all inputs from this generation before either call. Flags are
 * the full pre-ROI color flags; stage1 is the ROI/opening output. */
int LiveLocateHalfState(const VisionFrameView *full, const VisionFrameView *half,
                        uint32_t roi[8], uint32_t bounds[4], LiveRoiState *state);
int LiveDetectPlanes(const VisionFrameView *half, const uint8_t *flags,
                     const uint8_t *stage1, uint32_t plane_bytes,
                     const uint32_t roi[8], LiveResult *result);
#endif
void LiveDraw(uint8_t *bgr, uint32_t stride, const LiveResult *result);
#endif
