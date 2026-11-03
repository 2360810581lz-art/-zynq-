#ifndef LIVE_PROFILE_H
#define LIVE_PROFILE_H
/* Diagnostic only. Disabled hooks evaluate no arguments and emit no code. */
#ifdef LIVE_DETECT_PROFILE
#include "detector.h"
typedef struct { uint64_t sum, min, max; } LiveProfileMetric;
enum {
    LIVE_SYNC_ACQUIRE_PARK, LIVE_SYNC_ACQUIRE_COMPLETE, LIVE_SYNC_SOURCE_CACHE,
    LIVE_SYNC_COPY_DRAW_FLUSH, LIVE_SYNC_PUBLISH_PARK, LIVE_SYNC_PUBLISH_COMPLETE,
    LIVE_SYNC_COUNT
};
typedef struct {
    uint32_t samples, first_frame, last_frame, reserved;
    uint64_t first_ms, last_ms;
    uint32_t counts[9], modes[4], padding;
    LiveProfileMetric metric[9]; /* five frontend stages, three CCL stages, total */
    LiveProfileMetric sync[7]; /* six disjoint sync stages, whole frame */
} LiveProfileWindow;
typedef struct {
    uint32_t magic, version, sequence, windows;
    uint64_t tick_hz;
    uint32_t phase_errors, failed, paused, mode1, mode3, settling, recoveries, other;
    uint64_t failed_ticks;
    LiveProfileMetric timer;
    uint32_t timer_reads, reserved;
    LiveProfileWindow window[2];
} LiveDetectProfile;
extern volatile LiveDetectProfile live_detect_profile;
void LiveProfileInit(void);
void LiveProfileFrameBegin(void);
void LiveProfileSyncBegin(unsigned stage);
void LiveProfileSyncEnd(unsigned stage);
void LiveProfileFrameEnd(void);
void LiveProfileBegin(void);
void LiveProfileMark(void);
void LiveProfileEnd(int rc);
void LiveProfileObserve(int rc, const LiveResult *result, const LiveRoiState *roi,
                        uint32_t frame, uint64_t now_ms, uint32_t pause);
void LiveProfileRecovery(void);
void LiveProfilePublish(void);
#else
#define LiveProfileInit() ((void)0)
#define LiveProfileFrameBegin() ((void)0)
#define LiveProfileSyncBegin(stage) ((void)0)
#define LiveProfileSyncEnd(stage) ((void)0)
#define LiveProfileFrameEnd() ((void)0)
#define LiveProfileBegin() ((void)0)
#define LiveProfileMark() ((void)0)
#define LiveProfileEnd(rc) ((void)0)
#define LiveProfileObserve(rc,result,roi,frame,now_ms,pause) ((void)0)
#define LiveProfileRecovery() ((void)0)
#define LiveProfilePublish() ((void)0)
#endif
#endif
