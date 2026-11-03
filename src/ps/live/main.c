/* Dynamic foreground boxes on the preserved camera bitstream. */
#include <string.h>
#include <stdlib.h>
#include "display_demo.h"
#include "display_ctrl/display_ctrl.h"
#include "i2c/ps_i2c.h"
#include "ov5640.h"
#include "xgpiops.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "sleep.h"
#include "detector.h"
#include "profile.h"
#include "xtime_l.h"

#define HORSIZE (1280*3)
#define VERSIZE 720
#define VGA_VDMA_ID XPAR_AXIVDMA_0_DEVICE_ID
#define DISP_VTC_ID XPAR_VTC_0_DEVICE_ID
#define DYNCLK_BASEADDR XPAR_AXI_DYNCLK_0_BASEADDR
static DisplayCtrl dispCtrl;
static XAxiVdma vdma;
static XIicPs ps_i2c0;
static XGpioPs Gpio;
static uint8_t frameBuf[DISPLAY_NUM_FRAMES][DEMO_MAX_FRAME] __attribute__((aligned(64)));
static uint8_t *pFrames[DISPLAY_NUM_FRAMES];

static XAxiVdma liveCameraDma;
static uint8_t liveCameraBuffer[2][DEMO_STRIDE*VERSIZE] __attribute__((aligned(64)));
static unsigned camera_index;
static unsigned camera_park_pending;
static uint64_t camera_generation;
/* JTAG writes the first word without CPU cache snooping. Reserve full cache
 * lines: partial invalidation or a neighboring dirty object could otherwise
 * write an old command back over a new request. */
volatile union {
    uint32_t value;
    uint8_t cache_lines[64];
} live_command __attribute__((aligned(64)));
/* Stable after sequence is even; read twice when sampling from JTAG. */
volatile uint32_t live_status[36] __attribute__((aligned(64)));
LiveResult live_result;
LiveRoiState live_roi, live_roi_before;
LiveResult live_shown;
LiveBoxState live_boxes, live_boxes_before;
/* Bounded consecutive-frame evidence; flushed only for paused diagnostics.
 * frame,count,valid,overflow,stamp(2),mode,misses,ROI(8),raw(32),shown(32). */
uint32_t live_box_trace[64][80];
static unsigned recovery_count;

#ifdef LIVE_EDGE_CAPTURE
/* Diagnostic only: stop on one changing edge, with the other three nearby.
 * Capture the checked source, rather than a later frame after a host request. */
static int extent_spike(uint64_t now_ms)
{
    const LiveResult *old=&live_boxes_before.shown;
    if(live_result.valid!=1 || old->valid!=1 || live_result.count>8 || old->count>8 ||
       live_result.count!=old->count ||
       now_ms<live_boxes_before.stamp_ms || now_ms-live_boxes_before.stamp_ms>300)return 0;
    for(unsigned k=0;k<8;k++)
        if(abs((int)live_result.roi[k]-(int)old->roi[k])>4)return 0;
    for(unsigned i=0;i<live_result.count;i++)for(unsigned j=0;j<old->count;j++) {
        unsigned large=0,near=0,peak=0;
        for(unsigned k=0;k<4;k++) {
            unsigned delta=abs((int)live_result.objects[i].bbox[k]-(int)old->objects[j].bbox[k]);
            near+=delta<=32;large+=delta>4;if(delta>peak)peak=delta;
        }
        if(near==4 && large==1 && peak>6)return 1;
    }
    return 0;
}
#endif

static int gpio_setup(void)
{
    XGpioPs_Config *cfg=XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
    if(!cfg || XGpioPs_CfgInitialize(&Gpio,cfg,cfg->BaseAddr)!=XST_SUCCESS)return -1;
    XGpioPs_SetDirectionPin(&Gpio,54,1);XGpioPs_SetOutputEnablePin(&Gpio,54,1);
    return 0;
}
static void color_bars(uint8_t *p)
{
    static const uint8_t bgr[8][3]={{255,255,255},{0,255,255},{255,255,0},{0,255,0},
        {255,0,255},{0,0,255},{255,0,0},{0,0,0}};
    for(unsigned y=0;y<VERSIZE;y++)for(unsigned x=0;x<HORSIZE/3;x++)
        memcpy(p+y*DEMO_STRIDE+3*x,bgr[x/160],3);
    Xil_DCacheFlushRange((INTPTR)p,DEMO_STRIDE*VERSIZE);__sync_synchronize();
}

static uint64_t milliseconds(void)
{
    XTime ticks;XTime_GetTime(&ticks);
    return ticks/(COUNTS_PER_SECOND/1000);
}
static void status_begin(void)
{
    ++live_status[31];
    Xil_DCacheFlushRange((INTPTR)live_status,sizeof(live_status));__sync_synchronize();
}
static void status_end(void)
{
    Xil_DCacheFlushRange((INTPTR)live_status,sizeof(live_status));__sync_synchronize();
    ++live_status[31];
    Xil_DCacheFlushRange((INTPTR)live_status,sizeof(live_status));__sync_synchronize();
}
static int camera_complete(void)
{
    const uint32_t base=XPAR_AXIVDMA_1_BASEADDR;
    Xil_Out32(base+0x34,0x1000);
    uint64_t started=milliseconds();
    while(milliseconds()-started<1000) {
        uint32_t sr=Xil_In32(base+0x34);
        if(sr&0xcff1) {
            xil_printf("CAMERA_FRAME_ERROR sr=%08x cr=%08x\r\n",sr,Xil_In32(base+0x30));
            return -1;
        }
        if(sr&0x1000)return 0;
        usleep(100);
    }
    return -1;
}
static int camera_configure(void)
{
    camera_park_pending=0;
    XAxiVdma_Reset(&liveCameraDma,XAXIVDMA_WRITE);
    uint64_t start=milliseconds();
    while(XAxiVdma_ResetNotDone(&liveCameraDma,XAXIVDMA_WRITE)) {
        if(milliseconds()-start>500)return -1;
    }
    XAxiVdma_DmaSetup setup;memset(&setup,0,sizeof(setup));
    setup.VertSizeInput=VERSIZE;setup.HoriSizeInput=HORSIZE;setup.Stride=DEMO_STRIDE;
    setup.EnableSync=1;setup.PointNum=1;setup.GenLockRepeat=1;
    for(int i=0;i<XAXIVDMA_MAX_FRAMESTORE;i++)setup.FrameStoreStartAddr[i]=(UINTPTR)liveCameraBuffer[i%2];
    Xil_DCacheFlushRange((INTPTR)liveCameraBuffer,sizeof(liveCameraBuffer));
    Xil_Out32(XPAR_AXIVDMA_1_BASEADDR+0x34,0xcff0|0x1000);
    if(XAxiVdma_DmaConfig(&liveCameraDma,XAXIVDMA_WRITE,&setup)!=XST_SUCCESS ||
       XAxiVdma_DmaSetBufferAddr(&liveCameraDma,XAXIVDMA_WRITE,setup.FrameStoreStartAddr)!=XST_SUCCESS ||
       XAxiVdma_ChannelSetFrmCnt(&liveCameraDma.WriteChannel,1,0)!=XST_SUCCESS ||
       XAxiVdma_DmaStart(&liveCameraDma,XAXIVDMA_WRITE)!=XST_SUCCESS ||
       XAxiVdma_StartParking(&liveCameraDma,0,XAXIVDMA_WRITE)!=XST_SUCCESS)return -1;
    camera_index=0;return 0;
}
static int acquire(VisionFrameView *view,unsigned completions)
{
    if(camera_generation==UINT64_MAX)return -1;
    unsigned held=camera_index,next=1-held;
    LiveProfileSyncBegin(LIVE_SYNC_ACQUIRE_PARK);
    if(!camera_park_pending &&
       XAxiVdma_StartParking(&liveCameraDma,next,XAXIVDMA_WRITE)!=XST_SUCCESS)return -1;
    uint64_t started=milliseconds();
    while(XAxiVdma_CurrFrameStore(&liveCameraDma,XAXIVDMA_WRITE)!=next) {
        if(milliseconds()-started>1000)return -1;
        usleep(100);
    }
    LiveProfileSyncEnd(LIVE_SYNC_ACQUIRE_PARK);
    /* Continuous sensor stream; only the alternate parked buffer is written.
     * One fresh completion retires the old buffer for ordinary frames. Keep
     * two for paused readback, where the baseline diagnostic wants extra
     * retirement margin before the source is copied through JTAG. */
    LiveProfileSyncBegin(LIVE_SYNC_ACQUIRE_COMPLETE);
    if(completions<1)completions=1;
    for(unsigned i=0;i<completions;i++)if(camera_complete())return -1;
    LiveProfileSyncEnd(LIVE_SYNC_ACQUIRE_COMPLETE);
    LiveProfileSyncBegin(LIVE_SYNC_SOURCE_CACHE);
    camera_park_pending=0;camera_index=next;
    Xil_DCacheInvalidateRange((INTPTR)liveCameraBuffer[held],sizeof(liveCameraBuffer[held]));
    __sync_synchronize();
    *view=(VisionFrameView){liveCameraBuffer[held],HORSIZE/3,VERSIZE,DEMO_STRIDE,sizeof(liveCameraBuffer[held]),++camera_generation};
    LiveProfileSyncEnd(LIVE_SYNC_SOURCE_CACHE);
    return 0;
}
static int camera_request_next(void)
{
    /* Caller has finished every source read and flushed the separate output.
     * Only request the switch here; acquire still confirms the new park index
     * and a fresh completion before allowing CPU access to the other buffer. */
    if(XAxiVdma_StartParking(&liveCameraDma,1-camera_index,XAXIVDMA_WRITE)!=XST_SUCCESS)return -1;
    camera_park_pending=1;
    return 0;
}
static int camera_start(void)
{
    for(int attempt=0;attempt<5;attempt++) {
        /* Both receiver IPs expose CR bit0=reset, bit1=enable. Reset their
         * packet/lane state as well as the writer before restarting the sensor.
         * Sensor/VDMA reset alone leaves the baseline receiver out of sync. */
        Xil_Out32(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR,1);
        Xil_Out32(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR,1);
        if((Xil_In32(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR)&3)!=1 ||
           (Xil_In32(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR)&3)!=1)return -1;
        usleep(1000);
        XGpioPs_WritePin(&Gpio,54,0);usleep(1000000);
        XGpioPs_WritePin(&Gpio,54,1);usleep(1000000);
        if(i2c_init(&ps_i2c0,XPAR_XIICPS_0_DEVICE_ID,100000)!=XST_SUCCESS)continue;
        Xil_Out32(XPAR_AXI_GAMMACORRECTION_0_BASEADDR,3);
        /* Drain the receiver during sensor configuration: the sensor cannot
         * obey downstream backpressure and otherwise overflows its packet FIFO. */
        if(camera_configure())continue;
        Xil_Out32(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR,2);
        Xil_Out32(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR,2);
        if((Xil_In32(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR)&3)!=2 ||
           (Xil_In32(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR)&3)!=2)return -1;
        /* The receiver must observe the sensor wake handshake. */
        if(sensor_init(&ps_i2c0)!=XST_SUCCESS)continue;
        /* Acknowledge only the observed initialization frame-size condition,
         * once, without resetting an active stream. Reject bus/halt errors and
         * an internal error without FSizeMore; subsequent frames stay strict. */
        const uint32_t startup_errors=XAXIVDMA_SR_ERR_INTERNAL_MASK|XAXIVDMA_SR_ERR_FSZ_MORE_MASK;
        uint32_t sr=Xil_In32(XPAR_AXIVDMA_1_BASEADDR+0x34);
        xil_printf("CAMERA_STARTUP sr=%08x\r\n",sr);
        if(!(Xil_In32(XPAR_AXIVDMA_1_BASEADDR+0x30)&1U))continue;
        if((sr&0xcff1)&~(startup_errors|XAXIVDMA_IXR_ERROR_MASK))continue;
        if((sr&XAXIVDMA_SR_ERR_INTERNAL_MASK) && !(sr&XAXIVDMA_SR_ERR_FSZ_MORE_MASK))continue;
        Xil_Out32(XPAR_AXIVDMA_1_BASEADDR+0x34,startup_errors|XAXIVDMA_IXR_ALL_MASK);
        int warmup;
        for(warmup=0;warmup<30;warmup++)if(camera_complete())break;
        if(warmup==30)return 0;
        xil_printf("CAMERA_WARMUP_FAILED after=%d\r\n",warmup);
    }
    return -1;
}
static void camera_wait_start(void)
{
    /* Keep each hardware attempt bounded, but retry transient startup failures
     * in later batches. camera_start() requires fresh completed frames. */
    while(camera_start()) {
        xil_printf("LIVE_CAMERA_RETRY after 1 second\r\n");
        usleep(1000000);
    }
}
static int display_complete(void)
{
    const uint32_t base=XPAR_AXIVDMA_0_BASEADDR;
    Xil_Out32(base+4,0x1000);
    uint32_t started=milliseconds();
    while((uint32_t)(milliseconds()-started)<500) {
        uint32_t sr=Xil_In32(base+4);
        if(sr&0x8ff1)return -1;
        if(sr&0x1000)return 0;
        usleep(100);
    }
    return -1;
}
static int publish(unsigned index,unsigned completions)
{
    LiveProfileSyncBegin(LIVE_SYNC_PUBLISH_PARK);
    if(DisplayChangeFrame(&dispCtrl,index)!=XST_SUCCESS)return -1;
    uint32_t started=milliseconds();
    while(XAxiVdma_CurrFrameStore(&vdma,XAXIVDMA_READ)!=index) {
        if((uint32_t)(milliseconds()-started)>=500)return -1;
        usleep(100);
    }
    LiveProfileSyncEnd(LIVE_SYNC_PUBLISH_PARK);
    /* The park index has already advanced at a frame boundary above. Ordinary
     * live frames use the triple-buffer rotation, so waiting for another full
     * display completion only adds one HDMI refresh period to every frame.
     * Paused readback and recovery retain the two-completion retirement so
     * their DDR snapshots never race the display reader. */
    LiveProfileSyncBegin(LIVE_SYNC_PUBLISH_COMPLETE);
    if(completions>=2) {
        for(unsigned i=0;i<completions;i++)if(display_complete())return -1;
    }
    LiveProfileSyncEnd(LIVE_SYNC_PUBLISH_COMPLETE);
    return 0;
}
static uint32_t checksum(const uint8_t *p,uint32_t stride)
{
    uint32_t hash=2166136261U;
    for(unsigned y=0;y<VERSIZE;y++)for(unsigned x=0;x<HORSIZE;x++)hash=(hash^p[y*stride+x])*16777619U;
    return hash;
}
static int fail(uint32_t stage)
{
    status_begin();
    live_status[1]=stage;
    live_status[3]=live_status[4]=0;
    live_status[11]=Xil_In32(XPAR_AXIVDMA_1_BASEADDR+0x34);
    live_status[12]=Xil_In32(XPAR_AXIVDMA_0_BASEADDR+4);
    live_status[22]=stage;
    status_end();
    xil_printf("LIVE_ERROR stage=%d camera=%08x display=%08x\r\n",stage,live_status[11],live_status[12]);
    return -1;
}
static int recover_camera(uint32_t stage,unsigned index,uint32_t request)
{
    LiveProfileRecovery();
    memset(&live_boxes,0,sizeof(live_boxes));
    memset(live_box_trace,0,sizeof(live_box_trace));
    if(recovery_count<UINT32_MAX)++recovery_count;
    status_begin();
    live_status[1]=4;live_status[3]=live_status[4]=0;
    live_status[11]=Xil_In32(XPAR_AXIVDMA_1_BASEADDR+0x34);
    live_status[12]=Xil_In32(XPAR_AXIVDMA_0_BASEADDR+4);
    live_status[22]=stage;live_status[23]=recovery_count;live_status[24]=request;
    status_end();
    xil_printf("LIVE_CAMERA_RECOVERY %d stage=%d\r\n",recovery_count,stage);
    /* Clear old boxes while recovering; the display slot is still retired. */
    color_bars(pFrames[index]);
    if(publish(index,2))return -1;
    camera_wait_start();
    return 0;
}
int main(void)
{
    XAxiVdma_Config *cfg;
    LiveProfileInit();
    live_status[0]=0x4c495645;live_status[1]=1;
    Xil_DCacheFlushRange((INTPTR)live_status,sizeof(live_status));
    xil_printf("LIVE_BOOT dynamic_foreground_boxes; baseline PL\r\n");
    for(int i=0;i<DISPLAY_NUM_FRAMES;i++) {
        pFrames[i]=frameBuf[i];memset(pFrames[i],0,DEMO_MAX_FRAME);
        Xil_DCacheFlushRange((INTPTR)pFrames[i],DEMO_MAX_FRAME);
    }
    color_bars(pFrames[0]);
    if(gpio_setup())return fail(101);
    cfg=XAxiVdma_LookupConfig(VGA_VDMA_ID);
    if(!cfg || XAxiVdma_CfgInitialize(&vdma,cfg,cfg->BaseAddress)!=XST_SUCCESS)return fail(102);
    if(DisplayInitialize(&dispCtrl,&vdma,DISP_VTC_ID,DYNCLK_BASEADDR,pFrames,DEMO_STRIDE)!=XST_SUCCESS ||
       DisplayStart(&dispCtrl)!=XST_SUCCESS)return fail(103);
    cfg=XAxiVdma_LookupConfig(XPAR_AXIVDMA_1_DEVICE_ID);
    if(!cfg || XAxiVdma_CfgInitialize(&liveCameraDma,cfg,cfg->BaseAddress)!=XST_SUCCESS)return fail(104);
    camera_wait_start();
    unsigned index=1,frames=0,acquire_ms=0;
    for(;;) {
        Xil_DCacheInvalidateRange((INTPTR)&live_command,sizeof(live_command));
        uint32_t requested=live_command.value;
        if((requested&255U)==2) {
            live_command.value=0;Xil_DCacheFlushRange((INTPTR)&live_command,sizeof(live_command));
            if(recover_camera(112,index,requested))return fail(113);
            index=(index+1)%DISPLAY_NUM_FRAMES;
        }
        /* A late pause waits for the next frame so its source is checked
         * both before and after processing. Ordinary frames skip diagnostics. */
        LiveProfileFrameBegin();
        uint32_t pause_request=(requested&255U)==1?requested:0;
        uint32_t start=milliseconds(),t;
        VisionFrameView view;
        if(acquire(&view,pause_request?2:1)) {
            if(recover_camera(110,index,0))return fail(110);
            index=(index+1)%DISPLAY_NUM_FRAMES;
            continue;
        }
        acquire_ms=milliseconds()-start;start=milliseconds();
        uint32_t source_hash=0,source_hash_ms=0;
        if(pause_request) {
            source_hash=checksum(view.bgr,view.stride);
            source_hash_ms=milliseconds()-start;
        }
        t=milliseconds();
        live_roi_before=live_roi;live_roi_before.now_ms=milliseconds();
        live_roi=live_roi_before;
        LiveProfileBegin();
        int rc=LiveDetectState(&view,&live_result,&live_roi);
        LiveProfileEnd(rc);
        uint32_t detect_ms=milliseconds()-t;
        if(rc)memset(&live_result,0,sizeof(live_result));
        live_boxes_before=live_boxes;
        uint64_t box_stamp=milliseconds();
#ifdef LIVE_EDGE_CAPTURE
        /* Observe ordinary acquisition cadence until an event occurs. The
         * held source remains retired until camera_request_next(), so it is
         * still available for both checksums and readback on this frame. */
        if((requested&255U)==3 && extent_spike(box_stamp)) {
            pause_request=requested;
            uint32_t hash_start=milliseconds();
            source_hash=checksum(view.bgr,view.stride);
            source_hash_ms=milliseconds()-hash_start;
        }
#endif
        LiveStabilize(&live_result,&live_shown,&live_boxes,box_stamp);
        t=milliseconds();
        LiveProfileSyncBegin(LIVE_SYNC_COPY_DRAW_FLUSH);
        for(unsigned y=0;y<view.height;y++)memcpy(pFrames[index]+y*DEMO_STRIDE,view.bgr+y*view.stride,HORSIZE);
        LiveDraw(pFrames[index],DEMO_STRIDE,&live_shown);
        Xil_DCacheFlushRange((INTPTR)pFrames[index],sizeof(liveCameraBuffer[0]));__sync_synchronize();
        LiveProfileSyncEnd(LIVE_SYNC_COPY_DRAW_FLUSH);
        uint32_t draw_flush_ms=milliseconds()-t;t=milliseconds();
        /* Paused diagnostics keep the source immutable through hashes/JTAG.
         * A late pause is acknowledged only by the next fully checked frame. */
        if(!pause_request && camera_request_next()) {
            if(recover_camera(114,index,0))return fail(114);
            index=(index+1)%DISPLAY_NUM_FRAMES;
            continue;
        }
        if(publish(index,pause_request?2:1))return fail(108);
        uint32_t publish_ms=milliseconds()-t;t=milliseconds();
        uint32_t retired_hash_ms=0;
        if(pause_request) {
            Xil_DCacheInvalidateRange((INTPTR)view.bgr,view.bytes);
            if(checksum(view.bgr,view.stride)!=source_hash)return fail(111);
            retired_hash_ms=milliseconds()-t;
        }
        uint32_t processing_ms=milliseconds()-start;
        ++frames;
        uint32_t *trace=live_box_trace[frames%64];
        trace[0]=frames;trace[1]=live_result.count;trace[2]=live_result.valid;
        trace[3]=live_result.overflow;trace[4]=(uint32_t)box_stamp;
        trace[5]=(uint32_t)(box_stamp>>32);trace[6]=live_roi.mode;trace[7]=live_roi.misses;
        memcpy(trace+8,live_result.roi,sizeof(live_result.roi));
        for(unsigned i=0;i<8;i++) {
            memcpy(trace+16+4*i,live_result.objects[i].bbox,16);
            memcpy(trace+48+4*i,live_shown.objects[i].bbox,16);
        }
        Xil_DCacheInvalidateRange((INTPTR)&live_command,sizeof(live_command));
        status_begin();
        requested=live_command.value;
        int pause_ready=pause_request && requested==pause_request;
        if(pause_ready) {
            /* Pause with immutable source/output for independent byte review. */
            Xil_DCacheFlushRange((INTPTR)live_box_trace,sizeof(live_box_trace));
            live_status[1]=3;live_status[20]=source_hash;
            live_status[21]=checksum(pFrames[index],DEMO_STRIDE);
            live_status[24]=requested;
        }else {
            live_status[1]=2;live_status[20]=live_status[21]=0;
        }
        live_status[2]=frames;live_status[3]=live_result.count;live_status[4]=live_result.valid;
        live_status[5]=(uint32_t)view.generation;live_status[6]=(uint32_t)(view.generation>>32);
        live_status[7]=detect_ms;live_status[8]=(uint32_t)pFrames[index];live_status[9]=(uint32_t)view.bgr;
        live_status[10]=index;live_status[11]=Xil_In32(XPAR_AXIVDMA_1_BASEADDR+0x34);
        live_status[12]=Xil_In32(XPAR_AXIVDMA_0_BASEADDR+4);live_status[13]=(uint32_t)&live_result;
        live_status[14]=(uint32_t)rc;live_status[15]=processing_ms;live_status[16]=acquire_ms;
        live_status[19]=camera_index;
        live_status[25]=live_roi.mode;live_status[26]=live_roi.misses;
        live_status[27]=source_hash_ms;live_status[28]=draw_flush_ms;
        live_status[29]=publish_ms;live_status[30]=retired_hash_ms;
        live_status[32]=live_result.overflow;
        live_status[33]=(uint32_t)box_stamp;live_status[34]=(uint32_t)(box_stamp>>32);
        live_status[35]=1; /* current-frame box hysteresis readback protocol */
        uint64_t stamp=milliseconds();live_status[17]=(uint32_t)stamp;live_status[18]=(uint32_t)(stamp>>32);
        Xil_DCacheFlushRange((INTPTR)&live_result,sizeof(live_result));
        Xil_DCacheFlushRange((INTPTR)&live_roi_before,sizeof(live_roi_before));
        Xil_DCacheFlushRange((INTPTR)&live_roi,sizeof(live_roi));
        Xil_DCacheFlushRange((INTPTR)&live_shown,sizeof(live_shown));
        Xil_DCacheFlushRange((INTPTR)&live_boxes_before,sizeof(live_boxes_before));
        Xil_DCacheFlushRange((INTPTR)&live_boxes,sizeof(live_boxes));
        LiveProfileFrameEnd();
        LiveProfileObserve(rc,&live_result,&live_roi,frames,milliseconds(),pause_request);
        /* Publish before the even status sequence acknowledges a pause. The
         * frame timer excludes this final status commit and periodic UART. */
        if(pause_ready)LiveProfilePublish();
        status_end();
        if(frames==1 || frames%60==0)xil_printf("LIVE_FRAME %d boxes=%d detect_ms=%d\r\n",frames,live_result.count,detect_ms);
        while(pause_ready && live_command.value==pause_request) {
            usleep(1000);Xil_DCacheInvalidateRange((INTPTR)&live_command,sizeof(live_command));
        }
        index=(index+1)%DISPLAY_NUM_FRAMES;
    }
}
