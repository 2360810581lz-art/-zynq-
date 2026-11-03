#include "detector.h"
#include "profile.h"
#include "vision_black_border.h"
#include <stdlib.h>
#include <string.h>

#define W 640
#define H 360
static uint8_t small[W*H*3];
/* Full-resolution fitting is a cold/relocalization fallback after half-size
 * attempts fail. Normal frames verify cached edges without this workspace. */
static uint64_t border_work[(32*W*H+8+4*85*85+1920*41+7)/8];
static uint8_t mask[W*H], temp[W*H];
static uint32_t queue[W*H];
static uint8_t labels[W*H];
typedef struct {
    uint32_t area;int x0,y0,x1,y1,bx0,by0,bx1,by1;
    uint64_t members,near;
    uint32_t color_votes[6];
} Component;
static Component parts[64];

/* Shape extraction is deliberately bounded.  At the detector's half-size
 * resolution the reviewed objects have far fewer than 1024 boundary pixels;
 * taking every second boundary sample keeps the stack and runtime fixed even
 * for a large noisy candidate. */
#define SHAPE_POINT_CAP 1024
typedef struct { int x,y; } ShapePoint;
static ShapePoint shape_points[SHAPE_POINT_CAP],shape_hull[SHAPE_POINT_CAP];

/* Optional semantic-audit hook.  It is absent from every production build;
 * the isolated HLS comparison build defines LIVE_TRACE_MASK and copies the
 * three intermediate masks without changing detector behavior. */
#ifdef LIVE_TRACE_MASK
extern void LiveTraceMask(unsigned stage, const uint8_t *pixels);
#define LIVE_TRACE(stage, pixels) LiveTraceMask((stage), (pixels))
#else
#define LIVE_TRACE(stage, pixels) ((void)0)
#endif

/* Exact nearest-center index for each integer OpenCV-style hue [0,179].
 * Equal distances retain the first center, as in the original strict < loop. */
static const uint8_t hue_best[180]={
    1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,5,5,5,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

/* Return the OpenCV-style hue used by the reference color rules. */
static int bgr_hue(const uint8_t *p)
{
    int b=p[0],g=p[1],r=p[2],hi=b,lo=b;
    if(g>hi)hi=g;
    if(r>hi)hi=r;
    if(g<lo)lo=g;
    if(r<lo)lo=r;
    int diff=hi-lo;
    if(hi<40 || !diff)return -1;
    int numerator=hi==r?30*(g-b):hi==g?30*(b-r):30*(r-g);
    int hue=(numerator+(numerator>=0?diff/2:-diff/2))/diff;
    if(hi==g && hi!=r)hue+=60;
    else if(hi!=r && hi!=g)hue+=120;
    while(hue<0)hue+=180;
    while(hue>=180)hue-=180;
    return hue;
}

/* Stable IDs match vision_reference.py/default_config.json.  The board mask
 * intentionally groups red/orange together; this second pass separates them
 * from the component's actual hue statistics without changing segmentation. */
static int semantic_color(const uint8_t *p)
{
    int hue=bgr_hue(p);
    if(hue<0)return 0;
    if(hue<=7 || hue>=173)return 1; /* red */
    if(hue<=19)return 2;             /* orange */
    if(hue<=39)return 3;             /* yellow */
    if(hue<=80)return 4;             /* green */
    if(hue<=135)return 5;            /* blue */
    return 0;
}

typedef struct {int ax,ay,dx,dy,norm;} InsideEdge;
static void prepare_edges(const uint32_t *roi,InsideEdge *edge)
{
    for(int i=0;i<4;i++) {
        int j=(i+1)%4;InsideEdge *e=edge+i;
        e->ax=(int)roi[2*i]/2;e->ay=(int)roi[2*i+1]/2;
        e->dx=(int)roi[2*j]/2-e->ax;e->dy=(int)roi[2*j+1]/2-e->ay;
        e->norm=abs(e->dx)+abs(e->dy);
    }
}
static int inside_prepared(const InsideEdge *edge,int x,int y,int margin)
{
    for(int i=0;i<4;i++) {
        const InsideEdge *e=edge+i;
        if(e->dx*(y-e->ay)-e->dy*(x-e->ax)<margin*e->norm)return 0;
    }
    return 1;
}
static int edge_support(const uint32_t *roi)
{
    int good=0;
    for(int i=0;i<4;i++) {
        int j=(i+1)%4,hits=0;
        for(int n=2;n<30;n++) {
            int x=((32-n)*(int)roi[2*i]+n*(int)roi[2*j])/64;
            int y=((32-n)*(int)roi[2*i+1]+n*(int)roi[2*j+1])/64;
            int dark=0;
            for(int yy=y-4;yy<=y+4 && !dark;yy++)for(int xx=x-4;xx<=x+4;xx++) {
                if(xx<0 || xx>=W || yy<0 || yy>=H)continue;
                const uint8_t *p=small+3*(yy*W+xx);
                /* Match the border fitter's luma, with a five-level margin. */
                if(((3735u*p[0]+19235u*p[1]+9798u*p[2]+16384u)>>15)<115){dark=1;break;}
            }
            hits+=dark;
        }
        good+=hits>=15;
    }
    return good;
}
static int roi_visible(const uint32_t *roi);
/* The board is a nearly square planar target.  A false fit can still pass
 * roi_visible() when one long edge follows a dark background feature; reject
 * that fit before it enters the cache.  Keep this deliberately loose so
 * perspective and small camera movement remain valid (reviewed scenes are
 * close to 1.04, while the observed false fit is about 1.33). */
static int roi_shape_ok(const uint32_t *roi)
{
    uint64_t side[4];
    for(int i=0;i<4;i++) {
        int j=(i+1)%4;
        int64_t dx=(int64_t)roi[2*j]-(int64_t)roi[2*i];
        int64_t dy=(int64_t)roi[2*j+1]-(int64_t)roi[2*i+1];
        side[i]=(uint64_t)(dx*dx+dy*dy);
    }
    /* Compare squared lengths; 1.25 allows about 12% length difference. */
    return side[0] && side[1] &&
           side[0] <= 5*side[2]/4 && side[2] <= 5*side[0]/4 &&
           side[1] <= 5*side[3]/4 && side[3] <= 5*side[1]/4;
}
static int roi_candidate_ok(const uint32_t *roi)
{
    return roi_shape_ok(roi) && roi_visible(roi);
}
static int frame_visible(void)
{
    unsigned bright=0,total=0;
    for(int y=8;y<H;y+=16)for(int x=8;x<W;x+=16) {
        const uint8_t *p=small+3*(y*W+x);
        bright+=p[0]>=100 || p[1]>=100 || p[2]>=100;++total;
    }
    return bright*5>=total;
}
static int roi_visible(const uint32_t *roi)
{
    InsideEdge edges[4];prepare_edges(roi,edges);
    unsigned bright=0,lowlight=0,total=0;
    for(int y=6;y<H;y+=12)for(int x=6;x<W;x+=12)if(inside_prepared(edges,x,y,6)) {
        const uint8_t *p=small+3*(y*W+x);
        int hi=p[0];
        if(p[1]>hi)hi=p[1];
        if(p[2]>hi)hi=p[2];
        bright+=hi>=140;
        lowlight+=hi>=80;
        ++total;
    }
    /* Keep the historical bright-scene test, but accept a board whose white
     * interior is dimmed below 140 by the camera exposure. frame_visible()
     * still rejects an absent/black frame before this check. */
    return total>=40 && (bright*4>=total || lowlight*4>=total);
}
static int fallback_top(const uint32_t *roi)
{
    int hits=0,gap=0;
    for(int n=2;n<30;n++) {
        int x=((32-n)*(int)roi[0]+n*(int)roi[2])/64;
        int y=((32-n)*(int)roi[1]+n*(int)roi[3])/64,dark=0;
        /* A clipped evidence band can borrow darkness from the image edge. */
        if(x<8 || x>=W-8 || y<8 || y>=H-8)return 0;
        /* The fitted inner top edge can lie 15 full-size pixels below tape. */
        for(int yy=y-8;yy<=y+8 && !dark;yy++)for(int xx=x-8;xx<=x+8;xx++) {
            const uint8_t *p=small+3*(yy*W+xx);
            if(((3735u*p[0]+19235u*p[1]+9798u*p[2]+16384u)>>15)<130){dark=1;break;}
        }
        hits+=dark;gap=dark?0:gap+1;
        if(gap>3)return 0;
    }
    return hits>=21;
}
/* Failure-only verification of four finite tape segments. Three adjacent
 * low-luma pixels reject isolated speckles; longest-gap and total-support
 * guards reject a missing side. All geometry and top guards still apply. */
static int fallback_segments(const VisionFrameView *full,const uint32_t *roi)
{
    for(int i=0;i<4;i++) {
        int next=(i+1)%4,x0=(int)roi[2*i],y0=(int)roi[2*i+1];
        int dx=(int)roi[2*next]-x0,dy=(int)roi[2*next+1]-y0;
        int steps=abs(dx)>abs(dy)?abs(dx):abs(dy);
        if(steps<20)return 0;
        int first=(8*steps)/100,last=(92*steps)/100;
        int hits=0,gap=0,longest=0,total=last-first+1;
        for(int j=first;j<=last;j++) {
            int x=(int)(x0+(double)dx*j/steps+0.5);
            int y=(int)(y0+(double)dy*j/steps+0.5);
            int run=0,found=0;
            for(int offset=-16;offset<=16;offset++) {
                int xx=x+(i%2?offset:0),yy=y+(i%2?0:offset);
                if(xx<0 || xx>=1280 || yy<0 || yy>=720)return 0;
                const uint8_t *p=full->bgr+yy*full->stride+3*xx;
                int dark=((3735u*p[0]+19235u*p[1]+9798u*p[2]+16384u)>>15)<150;
                run=dark?run+1:0;
                if(run>=3){found=1;break;}
            }
            if(found){++hits;gap=0;}
            else{++gap;if(gap>longest)longest=gap;}
        }
        if(hits*5<total*4 || longest*100>total*12)return 0;
    }
    return 1;
}
static int locate_roi(const VisionFrameView *half,const VisionFrameView *full,uint32_t *roi)
{
    VisionBlackBorderSettings settings;
    VisionBlackBorderResult border;
    VisionBlackBorder_Defaults(&settings);
    settings.close_kernel=5; settings.interior_close_kernel=11;
    settings.interior_dilation_kernel=3; settings.corner_radius=3; settings.bottom_inset=2.5;
    int rc=VisionBlackBorder_Detect(half,&settings,border_work,sizeof(border_work),&border);
    if(!rc) {
        for(int i=0;i<8;i++)roi[i]=(uint32_t)(2*border.corners[i]+0.5);
        if(roi_candidate_ok(roi))return 0;
    }
    /* The camera exposure can leave the white board below the normal
     * interior threshold. Retry only after the normal calibration fails so
     * bright scenes retain the original border selection and cost. */
    settings.dark_threshold=80;
    settings.interior_threshold=100;
    rc=VisionBlackBorder_Detect(half,&settings,border_work,sizeof(border_work),&border);
    if(!rc) {
        for(int i=0;i<8;i++)roi[i]=(uint32_t)(2*border.corners[i]+0.5);
        if(roi_candidate_ok(roi))return 0;
    }
    /* A dim interior can coexist with a border above 80. Retry at half size
     * before paying for the full-resolution fallback chain. */
    settings.dark_threshold=110;
    rc=VisionBlackBorder_Detect(half,&settings,border_work,sizeof(border_work),&border);
    if(!rc) {
        for(int i=0;i<8;i++)roi[i]=(uint32_t)(2*border.corners[i]+0.5);
        if(roi_candidate_ok(roi) && edge_support(roi)>=2)return 0;
    }
    /* A failed low-light retry can have a different error code than the
     * initial half-resolution attempt. Always preserve the established full
     * resolution fallback before giving up. */
    settings.dark_threshold=110; settings.interior_threshold=150;
    settings.close_kernel=9;settings.interior_close_kernel=21;
    settings.interior_dilation_kernel=5;settings.corner_radius=6;settings.bottom_inset=5;
    rc=VisionBlackBorder_Detect(full,&settings,border_work,sizeof(border_work),&border);
    if(!rc) {
        for(int i=0;i<8;i++)roi[i]=(uint32_t)(border.corners[i]+0.5);
        if(roi_candidate_ok(roi))return 0;
    }
    settings.dark_threshold=80; settings.interior_threshold=100;
    rc=VisionBlackBorder_Detect(full,&settings,border_work,sizeof(border_work),&border);
    if(!rc) {
        for(int i=0;i<8;i++)roi[i]=(uint32_t)(border.corners[i]+0.5);
        if(roi_candidate_ok(roi))return 0;
    }
    /* Exposure moves the apparent black-line threshold between frames. After
     * calibrated attempts fail, scan a narrow threshold band while retaining
     * the production morphology and the fitter's geometric checks. */
    settings.interior_threshold=150; settings.close_kernel=9;
    settings.interior_close_kernel=31; settings.interior_dilation_kernel=5;
    settings.corner_radius=6; settings.bottom_inset=5;
    for(settings.dark_threshold=130;settings.dark_threshold<=150;settings.dark_threshold+=5) {
        rc=VisionBlackBorder_Detect(full,&settings,border_work,sizeof(border_work),&border);
        if(!rc) {
            for(int i=0;i<8;i++)roi[i]=(uint32_t)(border.corners[i]+0.5);
            int fit_ok=1;
            for(int i=0;i<3;i++)
                if(border.residual[i]>2.5)fit_ok=0;
            if(fit_ok && fallback_top(roi) && roi_candidate_ok(roi) && fallback_segments(full,roi))return 0;
        }
    }
    return rc?rc:-2;
}
static int close_roi(const uint32_t *a,const uint32_t *b)
{
    for(int i=0;i<8;i++)if(abs((int)a[i]-(int)b[i])>24)return 0;
    return 1;
}
static int update_roi(const VisionFrameView *half,const VisionFrameView *full,LiveRoiState *state)
{
    if(!frame_visible()) {
        state->locked=state->mode=0;state->misses=0;return -2;
    }
    /* Also reject an invalid old cache instead of extending it from the
     * unaffected left/bottom tape or holding it during relocalization. */
    if(state->locked && !roi_shape_ok(state->roi))state->locked=0;
    if(state->locked && roi_visible(state->roi) && (edge_support(state->roi)>=2 || fallback_segments(full,state->roi))) {
        state->misses=0;state->mode=2;state->last_good_ms=state->now_ms;return 0;
    }
    uint32_t candidate[8];
    int expired=!state->locked || state->now_ms-state->last_good_ms>1200;
    if(state->misses<UINT32_MAX)++state->misses;
    if(!state->locked || expired || state->misses==1 || state->misses%8==0) {
        if(!locate_roi(half,full,candidate) &&
           (expired || close_roi(state->roi,candidate))) {
            memcpy(state->roi,candidate,sizeof(candidate));
            state->locked=1;state->misses=0;state->mode=1;
            state->last_good_ms=state->now_ms;return 0;
        }
    }
    if(!expired && roi_visible(state->roi)){state->mode=3;return 0;}
    state->locked=0;state->mode=0;return -2;
}
typedef struct {int x0,y0,x1,y1;} ScanBounds;
static ScanBounds scan_bounds(const uint32_t *roi)
{
    ScanBounds b={0,0,W-1,H-1};int x[4],y[4];
    for(int i=0;i<4;i++) {
        if(roi[2*i]>=2*W || roi[2*i+1]>=2*H)return b;
        x[i]=(int)roi[2*i]/2;y[i]=(int)roi[2*i+1]/2;
    }
    for(int i=0;i<4;i++) {
        int j=(i+1)%4,k=(i+2)%4;
        if((x[j]-x[i])*(y[k]-y[j])-(y[j]-y[i])*(x[k]-x[j])<=0)return b;
    }
    b.x0=b.x1=x[0];b.y0=b.y1=y[0];
    for(int i=1;i<4;i++) {
        if(x[i]<b.x0)b.x0=x[i];
        if(x[i]>b.x1)b.x1=x[i];
        if(y[i]<b.y0)b.y0=y[i];
        if(y[i]>b.y1)b.y1=y[i];
    }
    return b;
}


/* A valid green body can have a warm, fragmented face at hue 30..34.
 * Recover only connected support, bounded by half the body's own extent.
 * Original eligible yellow bodies and distinct green seeds are preserved. */
static int support_eligible(uint32_t area,int x0,int y0,int x1,int y1,uint32_t roi_area)
{
    int w=x1-x0+1,h=y1-y0+1;
    return area>=500 && (uint32_t)(w*h)*10<=roi_area*3 && w>=12 && h>=12 &&
           w<=4*h && h<=4*w && area*5>=(uint32_t)(w*h);
}
/* Weak hue support must have local area, not a one-pixel bridge through paper.
 * Count the center and its neighbors in the original green/weak support set.
 * Original green pixels, including thin edges and tips, are always retained. */
static int green_weak_dense(int i)
{
    int x=i%W,y=i/W;unsigned support=0;
    for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++) {
        int xx=x+dx,yy=y+dy;
        if(xx<0 || xx>=W || yy<0 || yy>=H)continue;
        int n=yy*W+xx;
        support+=mask[n]==3 || ((labels[n]&1) && !(labels[n]&2));
    }
    return support>=4;
}
static void green_support(const uint32_t *roi,const ScanBounds *bounds)
{
    uint32_t roi_area=0;unsigned seeds=0;
    InsideEdge edges[4];prepare_edges(roi,edges);
    for(int i=0;i<4;i++) {
        int j=(i+1)%4;
        roi_area+=(roi[2*i]/2)*(roi[2*j+1]/2)-(roi[2*j]/2)*(roi[2*i+1]/2);
    }
    roi_area/=2;memset(temp,0,sizeof(temp));
    /* Analyze all original components before changing any mask pixel. */
    for(int sy=bounds->y0;sy<=bounds->y1;sy++)for(int sx=bounds->x0;sx<=bounds->x1;sx++) {
        int start=sy*W+sx,color=mask[start];
        if((color!=2 && color!=3) || (labels[start]&16))continue;
        uint32_t head=0,tail=0;int x0=sx,y0=sy,x1=sx,y1=sy;
        queue[tail++]=(uint32_t)start;labels[start]|=16;
        while(head<tail) {
            int i=(int)queue[head++],x=i%W,y=i/W;
            if(x<x0)x0=x;
            if(x>x1)x1=x;
            if(y<y0)y0=y;
            if(y>y1)y1=y;
            for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++) {
                int xx=x+dx,yy=y+dy;
                if(xx<0 || xx>=W || yy<0 || yy>=H)continue;
                int n=yy*W+xx;
                if(mask[n]==color && !(labels[n]&16)) {
                    labels[n]|=16;queue[tail++]=(uint32_t)n;
                }
            }
        }
        if(!support_eligible(tail,x0,y0,x1,y1,roi_area))continue;
        if(color==2)for(uint32_t n=0;n<tail;n++)labels[queue[n]]|=2;
        else if(seeds<64) {
            Component *c=&parts[seeds++];c->area=tail;
            c->x0=x0;c->y0=y0;c->x1=x1;c->y1=y1;
            for(uint32_t n=0;n<tail;n++)temp[queue[n]]=(uint8_t)seeds;
        }
    }
    for(unsigned id=1;id<=seeds;id++) {
        const Component *c=&parts[id-1];
        int rx=(c->x1-c->x0+2)/2,ry=(c->y1-c->y0+2)/2;
        int left=c->x0-rx,right=c->x1+rx,top=c->y0-ry,bottom=c->y1+ry;
        uint32_t head=0,tail=0;int ambiguous=0,x0=W,y0=H,x1=0,y1=0;
        for(int y=c->y0;y<=c->y1;y++)for(int x=c->x0;x<=c->x1;x++) {
            int i=y*W+x;
            if(temp[i]==id && !(labels[i]&8)){labels[i]|=8;queue[tail++]=(uint32_t)i;}
        }
        if(!tail)continue;
        while(head<tail) {
            int i=(int)queue[head++],x=i%W,y=i/W;
            if(x<x0)x0=x;
            if(x>x1)x1=x;
            if(y<y0)y0=y;
            if(y>y1)y1=y;
            for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++) {
                int xx=x+dx,yy=y+dy;
                if(xx<0 || xx>=W || yy<0 || yy>=H)continue;
                int n=yy*W+xx;
                if(temp[n] && temp[n]!=id){ambiguous=1;continue;}
                if(xx<left || xx>right || yy<top || yy>bottom ||
                   !inside_prepared(edges,xx,yy,1) || (labels[n]&8))continue;
                if(mask[n]==3 || ((labels[n]&1) && !(labels[n]&2) && green_weak_dense(n))) {
                    labels[n]|=8;queue[tail++]=(uint32_t)n;
                }
            }
        }
        /* Keep original masks when weak support reaches another green body
         * or would fail the established component geometry. */
        if(!ambiguous && support_eligible(tail,x0,y0,x1,y1,roi_area))
            for(uint32_t n=0;n<tail;n++) {
                mask[queue[n]]=3;temp[queue[n]]=(uint8_t)id;
            }
    }
}

static void foreground(const uint32_t *roi,const ScanBounds *bounds)
{
    InsideEdge edges[4];prepare_edges(roi,edges);
    memset(mask,0,sizeof(mask));memset(labels,0,sizeof(labels));
    for(int y=bounds->y0;y<=bounds->y1;y++)for(int x=bounds->x0;x<=bounds->x1;x++) {
        int i=y*W+x;const uint8_t *p=small+3*i;
        int b=p[0],g=p[1],r=p[2],hi=b,lo=b,hue=0;
        if(g>hi)hi=g;
        if(r>hi)hi=r;
        if(g<lo)lo=g;
        if(r<lo)lo=r;
        int diff=hi-lo;
        /* Every hue group requires saturation >=20. Reject white background
         * before the per-pixel division and four-edge ROI test. */
        if(hi<40 || !diff || 255*diff+hi/2<20*hi || !inside_prepared(edges,x,y,1))continue;
        int numerator=hi==r?30*(g-b):hi==g?30*(b-r):30*(r-g);
        hue=(numerator+(numerator>=0?diff/2:-diff/2))/diff;
        if(hi==g && hi!=r)hue+=60;
        else if(hi!=r && hi!=g)hue+=120;
        /* Normalize after applying the channel offset.  Green/cyan pixels
         * whose B channel is below R produce a negative pre-offset hue; the
         * old order could leave it above 179 and index hue_best out of range. */
        while(hue<0)hue+=180;
        while(hue>=180)hue-=180;
        if(hue>=30 && hue<35)labels[i]=1;
        static const int centers[6]={7,179,13,23,60,110};
        static const int groups[6]={1,1,1,2,3,4},limits[6]={8,8,7,12,25,20};
        int best=hue_best[hue],distance=abs(hue-centers[best]);
        if(distance>90)distance=180-distance;
        /* A rejected nearest-yellow pixel can still meet the existing
         * green hue/chroma gates, including the low-chroma hue-35 edge. */
        if(best==3 && (distance>limits[best] || 255*diff+hi/2<50*hi) &&
           abs(hue-centers[4])<=limits[4]) {
            best=4;distance=abs(hue-centers[best]);
        }
        int group=groups[best];
        /* The board's low-light white surface has a warm color cast.  Keep
         * the red/orange foreground group separate from that cast with a
         * slightly stronger chroma floor; 82 still retains the weakest
         * orange/yellow samples in the reviewed history set. */
        int minimum=group==1?82:group==2?50:20;
        /* Under the board's low-light warm cast, pale yellow shifts toward
         * the orange hue centers.  Its red-green gap remains small; use that
         * channel relation only in the narrow overlap band. */
        if(group==1 && hue>=14 && hue<=20 && r-g<=24 && hi>=80) {
            group=2; minimum=50;
        }
        if(distance<=limits[best] && 255*diff+hi/2>=minimum*hi)mask[i]=(uint8_t)group;
    }
    LIVE_TRACE(0,mask);
    LiveProfileMark();
    /* Opening removes narrow shadow bridges for warm/yellow/blue groups. The
     * green object has a thin, low-saturation edge that gets erased by
     * erosion; retain its mask and let component gates
     * reject isolated noise instead. */
    memset(temp,0,sizeof(temp));
    int y0=bounds->y0<1?1:bounds->y0,y1=bounds->y1>H-2?H-2:bounds->y1;
    int x0=bounds->x0<1?1:bounds->x0,x1=bounds->x1>W-2?W-2:bounds->x1;
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++) {
        int i=y*W+x,c=mask[i],keep=c!=0;
        for(int dy=-1;dy<=1 && keep && c!=3;dy++)for(int dx=-1;dx<=1;dx++)
            if(mask[i+dy*W+dx]!=c){keep=0;break;}
        if(keep)temp[i]=(uint8_t)c;
    }
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++) {
        int i=y*W+x,c=mask[i],keep=0;
        if(!c)continue;
        for(int dy=-1;dy<=1 && !keep;dy++)for(int dx=-1;dx<=1;dx++)
            if(temp[i+dy*W+dx]==c){keep=1;break;}
        if(!keep)mask[i]=0;
    }
    LIVE_TRACE(1,mask);
    LiveProfileMark();
    green_support(roi,bounds);
    LIVE_TRACE(2,mask);
    LIVE_TRACE(3,labels);
    LiveProfileMark();
}
/* Pale bright reflections may attach to a real blue component. Preserve
 * dim outline connected to its strongest S>=40 seed; no frame history is used.
 * Original component geometry still controls eligibility and side merging. */
static void blue_bounds(Component *c,unsigned id,uint32_t original_core)
{
    uint32_t best_core=0,best_area=0;
    for(int sy=c->y0;sy<=c->y1;sy++)for(int sx=c->x0;sx<=c->x1;sx++) {
        int start=sy*W+sx;if(labels[start]!=id || !temp[start])continue;
        uint32_t head=0,tail=0,core=temp[start]==2;
        int x0=sx,y0=sy,x1=sx,y1=sy;
        queue[tail++]=(uint32_t)start;temp[start]=0;
        while(head<tail) {
            int i=(int)queue[head++],x=i%W,y=i/W;
            if(x<x0)x0=x;
            if(x>x1)x1=x;
            if(y<y0)y0=y;
            if(y>y1)y1=y;
            for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++) {
                int xx=x+dx,yy=y+dy;
                if(xx<c->x0 || xx>c->x1 || yy<c->y0 || yy>c->y1)continue;
                int n=yy*W+xx;
                if(labels[n]!=id || !temp[n])continue;
                core+=temp[n]==2;temp[n]=0;queue[tail++]=(uint32_t)n;
            }
        }
        if(core>best_core || (core==best_core && core && tail>best_area)) {
            best_core=core;best_area=tail;
            c->bx0=x0-2>c->x0?x0-2:c->x0;
            c->by0=y0-2>c->y0?y0-2:c->y0;
            c->bx1=x1+2<c->x1?x1+2:c->x1;
            c->by1=y1+2<c->y1?y1+2:c->y1;
        }
    }
    /* A substantial second seed could be another real face/object. Retain
     * original bounds when the selected branch owns less than 80% of core. */
    if(best_core*5<original_core*4) {
        c->bx0=c->x0;c->by0=c->y0;c->bx1=c->x1;c->by1=c->y1;
    }
}

/* Yellow shadows can survive 3x3 opening as thin attached paper branches.
 * Bound the 5x5-supported body with its erosion radius plus a two-half-pixel
 * tip allowance, clamped to the original component.
 * Preserve original bounds unless this retains at least 95% of the component;
 * eligibility, side merging and the foreground mask remain unchanged. */
static void yellow_bounds(Component *c,unsigned id)
{
    int x0=W,y0=H,x1=-1,y1=-1;
    for(int y=c->y0+2;y<=c->y1-2;y++)for(int x=c->x0+2;x<=c->x1-2;x++) {
        if(labels[y*W+x]!=id)continue;
        int solid=1;
        for(int dy=-2;dy<=2 && solid;dy++)for(int dx=-2;dx<=2;dx++)
            if(labels[(y+dy)*W+x+dx]!=id){solid=0;break;}
        if(!solid)continue;
        if(x<x0)x0=x;
        if(x>x1)x1=x;
        if(y<y0)y0=y;
        if(y>y1)y1=y;
    }
    if(x1<x0 || y1<y0)return;
    x0=x0-4>c->x0?x0-4:c->x0;y0=y0-4>c->y0?y0-4:c->y0;
    x1=x1+4<c->x1?x1+4:c->x1;y1=y1+4<c->y1?y1+4:c->y1;
    uint32_t retained=0;
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++)retained+=labels[y*W+x]==id;
    if(retained*20<c->area*19)return;
    c->bx0=x0;c->by0=y0;c->bx1=x1;c->by1=y1;
}

static int component_has(const Component *c,int x,int y)
{
    if(x<0 || x>=W || y<0 || y>=H)return 0;
    unsigned id=labels[y*W+x];
    return id>=1 && id<=64 && (c->members&(1ULL<<(id-1)))!=0;
}

static int shape_point_compare(const void *aa,const void *bb)
{
    const ShapePoint *a=(const ShapePoint *)aa,*b=(const ShapePoint *)bb;
    if(a->x!=b->x)return a->x-b->x;
    return a->y-b->y;
}

static int shape_cross(const ShapePoint *a,const ShapePoint *b,const ShapePoint *c)
{
    return (int)((int64_t)(b->x-a->x)*(c->y-a->y)-
                 (int64_t)(b->y-a->y)*(c->x-a->x));
}

/* Match the reference classifier's coarse polygon/circularity gates using
 * the binary component silhouette.  This is a conservative integer estimate:
 * unknown is preferred when the sampled hull does not support one of the four
 * configured shapes. */
static unsigned component_shape(const Component *c,unsigned *confidence)
{
    int x0=c->x0,y0=c->y0,x1=c->x1,y1=c->y1;
    int width=x1-x0+1,height=y1-y0+1;
    uint64_t perimeter=0,boundary_seen=0;
    unsigned point_count=0;
    int row_min[2]={W,W},row_max[2]={-1,-1};
    int row_y[2]={y0+(height-1)*15/100,y0+(height-1)*85/100};
    int row_band=height*4/100;if(row_band<1)row_band=1;
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++) {
        if(!component_has(c,x,y))continue;
        for(int r=0;r<2;r++)if(abs(y-row_y[r])<=row_band) {
            if(x<row_min[r])row_min[r]=x;
            if(x>row_max[r])row_max[r]=x;
        }
        int edge=0;
        edge+=!component_has(c,x-1,y);edge+=!component_has(c,x+1,y);
        edge+=!component_has(c,x,y-1);edge+=!component_has(c,x,y+1);
        perimeter+=(uint64_t)edge;
        if(edge) {
            /* Every second boundary pixel is enough to retain the corners. */
            if(!(boundary_seen&1u) && point_count<SHAPE_POINT_CAP) {
                shape_points[point_count++]=(ShapePoint){x,y};
            }
            ++boundary_seen;
        }
    }
    *confidence=0;
    if(point_count<3 || perimeter==0)return 0;
    qsort(shape_points,point_count,sizeof(shape_points[0]),shape_point_compare);
    int hull_count=0;
    for(unsigned i=0;i<point_count;i++) {
        while(hull_count>=2 && shape_cross(&shape_hull[hull_count-2],
                                           &shape_hull[hull_count-1],
                                           &shape_points[i])<=0)--hull_count;
        shape_hull[hull_count++]=shape_points[i];
    }
    int lower_end=hull_count;
    for(int i=(int)point_count-2;i>=0;i--) {
        while(hull_count>lower_end && shape_cross(&shape_hull[hull_count-2],
                                                  &shape_hull[hull_count-1],
                                                  &shape_points[i])<=0)--hull_count;
        shape_hull[hull_count++]=shape_points[i];
    }
    if(hull_count>1)--hull_count;
    if(hull_count<3)return 0;
    int64_t hull_area2=0;
    for(int i=0;i<hull_count;i++) {
        int j=(i+1)%hull_count;
        hull_area2+=(int64_t)shape_hull[i].x*shape_hull[j].y-
                    (int64_t)shape_hull[j].x*shape_hull[i].y;
    }
    uint64_t hull_area=(uint64_t)(hull_area2<0?-hull_area2:hull_area2)/2u;
    if(hull_area==0)return 0;
    uint64_t area=c->area;
    uint64_t rect_area=(uint64_t)width*(uint64_t)height;
    uint64_t perimeter2=perimeter*perimeter;
    uint64_t circularity_num=area*12566u; /* 4*pi scaled by 1000 */
    uint64_t solidity_num=area*100u/hull_area;
    uint64_t rectangularity_num=rect_area?area*100u/rect_area:0;
    int top_width=row_max[0]>=row_min[0]?row_max[0]-row_min[0]+1:0;
    int bottom_width=row_max[1]>=row_min[1]?row_max[1]-row_min[1]+1:0;
    (void)top_width;(void)bottom_width;

    unsigned shape=0,conf=0;
    /* Same precedence and IDs as vision_reference.classify_shape(). */
    if(hull_count==3 && solidity_num>=80u) {
        shape=2;conf=166u+(unsigned)(solidity_num>100?100:solidity_num)*64u/100u;
    } else if(hull_count==5 && solidity_num>=75u) {
        shape=3;conf=166u+(unsigned)(solidity_num>100?100:solidity_num)*64u/100u;
    } else if(hull_count>=7 && circularity_num*1000u>=650u*perimeter2) {
        shape=1;conf=140u+(unsigned)((circularity_num>1000u?1000u:circularity_num)*115u/1000u);
    } else if(hull_count==4 && rectangularity_num>=70u) {
        shape=4;conf=140u+(unsigned)(rectangularity_num>100?100:rectangularity_num)*102u/100u;
    }
    if(conf>255u)conf=255u;
    *confidence=conf;
    return shape;
}

static uint32_t component_tag(const Component *c)
{
    unsigned color=0,color_votes=0,total=0;
    for(unsigned i=1;i<=5;i++) {
        total+=c->color_votes[i];
        if(c->color_votes[i]>color_votes) {
            color=i;color_votes=c->color_votes[i];
        }
    }
    unsigned shape=0,shape_conf=0;
    if(color==0 || total==0)color=0;
    unsigned color_conf=total?(color_votes*255u/total):0;
    shape=component_shape(c,&shape_conf);
    unsigned valid=color!=0 && shape!=0;
    unsigned confidence=shape_conf<color_conf?shape_conf:color_conf;
    return LIVE_TAG_PACK(color,shape,valid,confidence);
}

static void components(LiveResult *out,const ScanBounds *bounds)
{
    unsigned count=0;
    uint32_t roi_area=0;
    for(int i=0;i<4;i++) {
        int j=(i+1)%4;
        roi_area+=(out->roi[2*i]/2)*(out->roi[2*j+1]/2)-
                  (out->roi[2*j]/2)*(out->roi[2*i+1]/2);
    }
    roi_area/=2;
    memset(labels,0,sizeof(labels));memset(parts,0,sizeof(parts));
    uint32_t areas[8]={0};
    for(int sy=bounds->y0;sy<=bounds->y1;sy++)for(int sx=bounds->x0;sx<=bounds->x1;sx++) {
        int start=sy*W+sx;if(!mask[start])continue;
        uint32_t head=0,tail=0,blue_core=0,core_v[256]={0},color_votes[6]={0};int x0=W,y0=H,x1=0,y1=0,color=mask[start];
        queue[tail++]=(uint32_t)start;mask[start]=0;
        while(head<tail) {
            int i=(int)queue[head++],x=i%W,y=i/W;
            int semantic=semantic_color(small+3*i);
            if(semantic>0)++color_votes[semantic];
            if(color==4) {
                const uint8_t *p=small+3*i;
                unsigned hi=p[0],lo=p[0];
                for(int k=1;k<3;k++){if(p[k]>hi)hi=p[k];if(p[k]<lo)lo=p[k];}
                unsigned core=255u*(hi-lo)+hi/2>=40u*hi;
                blue_core+=core;core_v[hi]+=core;
                /* Keep every weak blue pixel until the component's adaptive
                 * V limit is known; the fixed 210 ceiling made the result
                 * depend on exposure and left a pale attached branch. */
                temp[i]=(uint8_t)(core?2:1);
            }
            if(x<x0)x0=x;
            if(x>x1)x1=x;
            if(y<y0)y0=y;
            if(y>y1)y1=y;
            for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++) {
                int xx=x+dx,yy=y+dy;
                if(xx<0 || xx>=W || yy<0 || yy>=H)continue;
                int n=yy*W+xx;
                if(mask[n]==color){mask[n]=0;queue[tail++]=(uint32_t)n;}
            }
        }
        int w=x1-x0+1,h=y1-y0+1;
        /* Retain the low-saturation blue outline, but reject pale reflections
         * without a color core: at least 20% of the component must have S>=40. */
        if(tail<500 || (uint32_t)(w*h)*10>roi_area*3 || w<12 || h<12 || w>4*h || h>4*w ||
           tail*5<(uint32_t)(w*h) || (color==4 && blue_core*5<tail))continue;
        if(count==64){++out->overflow;continue;}
        Component *c=&parts[count];
        c->area=tail;c->x0=x0;c->y0=y0;c->x1=x1;c->y1=y1;c->members=1ULL<<count;
        memcpy(c->color_votes,color_votes,sizeof(c->color_votes));
        for(uint32_t i=0;i<tail;i++)labels[queue[i]]=(uint8_t)(count+1);
        c->bx0=x0;c->by0=y0;c->bx1=x1;c->by1=y1;
        if(color==2)yellow_bounds(c,count+1);
        if(color==4) {
            /* Weak pixels above the 75th-percentile core value are usually
             * pale reflected background. Strong pixels,
             * branch selection and the 80% core guard remain unchanged. */
            uint32_t wanted=(blue_core*75+99)/100,seen=0;
            unsigned value=0;
            for(;value<255;value++){seen+=core_v[value];if(seen>=wanted)break;}
            unsigned limit=value;
            for(uint32_t n=0;n<tail;n++) {
                int i=(int)queue[n];if(temp[i]!=1)continue;
                const uint8_t *p=small+3*i;unsigned hi=p[0];
                if(p[1]>hi)hi=p[1];
                if(p[2]>hi)hi=p[2];
                if(hi>limit)temp[i]=0;
            }
            blue_bounds(c,count+1,blue_core);
        }
        ++count;
    }
    LiveProfileMark();
    uint32_t smallest=W*H+1,largest=0;
    for(unsigned i=0;i<count;i++) {
        if(parts[i].area<smallest)smallest=parts[i].area;
        if(parts[i].area>largest)largest=parts[i].area;
    }
    /* Every first merge requires 3*small_area<=large_area. If no initial
     * pair can satisfy it, nothing can merge or grow in later iterations.
     * Otherwise build the entire original graph, including all later pairs. */
    if(count>1 && largest>=3*smallest) {
        /* Only merge a small side fragment with a substantially larger touching
         * body. Two half-pixel distance is a narrow face seam; independent objects
         * with a gap this small can still be ambiguous and need real validation. */
        int y0=bounds->y0<2?2:bounds->y0,y1=bounds->y1>H-3?H-3:bounds->y1;
        int x0=bounds->x0<2?2:bounds->x0,x1=bounds->x1>W-3?W-3:bounds->x1;
        for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++) {
            int id=labels[y*W+x];if(!id)continue;
            for(int dy=-2;dy<=2;dy++)for(int dx=-2;dx<=2;dx++) {
                int other=labels[(y+dy)*W+x+dx];
                if(other && other!=id)parts[id-1].near|=1ULL<<(other-1);
            }
        }
    }
    LiveProfileMark();
    for(;;) {
        int from=-1,to=-1;
        for(unsigned i=0;i<count;i++)for(unsigned j=0;j<count;j++) {
            Component *a=&parts[i],*b=&parts[j];
            if(i==j || !a->area || !b->area || 3*a->area>b->area || !(a->near&b->members))continue;
            int ox=(a->x1<b->x1?a->x1:b->x1)-(a->x0>b->x0?a->x0:b->x0)+1;
            int oy=(a->y1<b->y1?a->y1:b->y1)-(a->y0>b->y0?a->y0:b->y0)+1;
            if(ox<0)ox=0;
            if(oy<0)oy=0;
            int aw=a->x1-a->x0+1,ah=a->y1-a->y0+1,bw=b->x1-b->x0+1,bh=b->y1-b->y0+1;
            int smaller=aw*ah<bw*bh?aw*ah:bw*bh;
            if(2*ox*oy<smaller && 5*ox<4*(aw<bw?aw:bw) && 5*oy<4*(ah<bh?ah:bh))continue;
            int x0=a->x0<b->x0?a->x0:b->x0,x1=a->x1>b->x1?a->x1:b->x1;
            int y0=a->y0<b->y0?a->y0:b->y0,y1=a->y1>b->y1?a->y1:b->y1;
            if((uint32_t)((x1-x0+1)*(y1-y0+1))*10>roi_area*3)continue;
            if(from<0 || a->area<parts[from].area){from=(int)i;to=(int)j;}
        }
        if(from<0)break;
        Component *a=&parts[from],*b=&parts[to];
        if(a->x0<b->x0)b->x0=a->x0;
        if(a->x1>b->x1)b->x1=a->x1;
        if(a->y0<b->y0)b->y0=a->y0;
        if(a->y1>b->y1)b->y1=a->y1;
        if(a->bx0<b->bx0)b->bx0=a->bx0;
        if(a->bx1>b->bx1)b->bx1=a->bx1;
        if(a->by0<b->by0)b->by0=a->by0;
        if(a->by1>b->by1)b->by1=a->by1;
        b->area+=a->area;b->members|=a->members;b->near|=a->near;
        for(unsigned k=0;k<6;k++)b->color_votes[k]+=a->color_votes[k];
        a->area=0;
    }
    for(unsigned n=0;n<count;n++)if(parts[n].area) {
        const Component *c=&parts[n];uint32_t tail=c->area;
        unsigned slot=out->count;
        if(slot>=8) {
            ++out->overflow;
            slot=0;for(unsigned i=1;i<8;i++)if(areas[i]<areas[slot])slot=i;
            if(tail<=areas[slot])continue;
        }else ++out->count;
        areas[slot]=tail;
        LiveObject *o=&out->objects[slot];
        o->bbox[0]=2*c->bx0;o->bbox[1]=2*c->by0;o->bbox[2]=2*c->bx1+1;o->bbox[3]=2*c->by1+1;
        o->center[0]=c->bx0+c->bx1;o->center[1]=c->by0+c->by1;
        o->match_error=component_tag(c);
    }
}
/* Same rounded 2x2 channel average; row pointers avoid repeated frame-field
 * loads in the scalar inner loop. Only active pixels are read. */
static void reduce_input(const VisionFrameView *f)
{
    const uint32_t stride=f->stride;
    for(int y=0;y<H;y++) {
        const uint8_t *a=f->bgr+2*y*stride,*b=a+stride;
        uint8_t *d=small+3*y*W;
        for(int x=0;x<W;x++,a+=6,b+=6,d+=3) {
            d[0]=(a[0]+a[3]+b[0]+b[3]+2)/4;
            d[1]=(a[1]+a[4]+b[1]+b[4]+2)/4;
            d[2]=(a[2]+a[5]+b[2]+b[5]+2)/4;
        }
    }
}
int LiveDetectState(const VisionFrameView *f,LiveResult *out,LiveRoiState *state)
{
    if(!out)return -1;
    memset(out,0,sizeof(*out));
    if(!state || !f || !f->bgr || !f->generation || f->width!=2*W || f->height!=2*H ||
       f->stride<6*W || (uint64_t)(f->height-1)*f->stride+6*W>f->bytes)return -1;
    if(state->generation && (f->generation<=state->generation || state->now_ms<state->last_good_ms))return -1;
    state->generation=f->generation;
    reduce_input(f);
    LiveProfileMark();
    VisionFrameView half={small,W,H,3*W,sizeof(small),f->generation};
    if(update_roi(&half,f,state))return -2;
    memcpy(out->roi,state->roi,sizeof(out->roi));
    ScanBounds bounds=scan_bounds(out->roi);
    LiveProfileMark();
    foreground(out->roi,&bounds);components(out,&bounds);out->valid=1;
    return 0;
}
int LiveDetect(const VisionFrameView *f,LiveResult *out)
{
    LiveRoiState state;memset(&state,0,sizeof(state));
    return LiveDetectState(f,out,&state);
}

#ifdef LIVE_PL_HANDOFF
static int prepared_view(const VisionFrameView *f,uint32_t width,uint32_t height)
{
    return f && f->bgr && f->generation && f->width==width && f->height==height &&
        f->stride>=3*width && (uint64_t)(height-1)*f->stride+3*width<=f->bytes;
}
static void prepared_half(const VisionFrameView *half)
{
    for(unsigned y=0;y<H;y++)memcpy(small+3*y*W,half->bgr+y*half->stride,3*W);
}
int LiveLocateHalfState(const VisionFrameView *full,const VisionFrameView *half,
                       uint32_t roi[8],uint32_t bounds[4],LiveRoiState *state)
{
    if(!roi || !bounds)return -1;
    memset(roi,0,8*sizeof(*roi));memset(bounds,0,4*sizeof(*bounds));
    if(!state || !prepared_view(full,2*W,2*H) || !prepared_view(half,W,H) ||
       full->generation!=half->generation || (state->generation &&
       (full->generation<=state->generation || state->now_ms<state->last_good_ms)))return -1;
    state->generation=full->generation;
    prepared_half(half);
    VisionFrameView view={small,W,H,3*W,sizeof(small),half->generation};
    if(update_roi(&view,full,state))return -2;
    memcpy(roi,state->roi,8*sizeof(*roi));
    ScanBounds b=scan_bounds(roi);
    bounds[0]=(uint32_t)b.x0;bounds[1]=(uint32_t)b.y0;
    bounds[2]=(uint32_t)b.x1;bounds[3]=(uint32_t)b.y1;
    return 0;
}
int LiveDetectPlanes(const VisionFrameView *half,const uint8_t *flags,
                     const uint8_t *stage1,uint32_t plane_bytes,
                     const uint32_t roi[8],LiveResult *out)
{
    if(!out)return -1;
    memset(out,0,sizeof(*out));
    if(!prepared_view(half,W,H) || !flags || !stage1 || plane_bytes!=W*H || !roi)return -1;
    for(unsigned i=0;i<4;i++) {
        if(roi[2*i]>=2*W || roi[2*i+1]>=2*H)return -1;
        unsigned j=(i+1)%4,k=(i+2)%4;
        int ax=(int)(roi[2*j]/2)-(int)(roi[2*i]/2);
        int ay=(int)(roi[2*j+1]/2)-(int)(roi[2*i+1]/2);
        int bx=(int)(roi[2*k]/2)-(int)(roi[2*j]/2);
        int by=(int)(roi[2*k+1]/2)-(int)(roi[2*j+1]/2);
        if(ax*by-ay*bx<=0)return -1;
    }
    // Check encoding without recomputing the HLS color/opening work.
    for(unsigned i=0;i<W*H;i++)if(flags[i]>15 || (flags[i]&7)>4 || stage1[i]>4)return -1;
    prepared_half(half);memcpy(mask,stage1,sizeof(mask));memset(labels,0,sizeof(labels));
    ScanBounds bounds=scan_bounds(roi);InsideEdge edges[4];prepare_edges(roi,edges);
    for(int y=bounds.y0;y<=bounds.y1;y++)for(int x=bounds.x0;x<=bounds.x1;x++) {
        int i=y*W+x;
        if(inside_prepared(edges,x,y,1))labels[i]=(flags[i]&8)?1:0;
    }
    memcpy(out->roi,roi,sizeof(out->roi));
    green_support(roi,&bounds);components(out,&bounds);out->valid=1;
    return 0;
}
#endif

static void point(uint8_t *p,uint32_t stride,int x,int y,int r,int g,int b)
{
    if(x<0 || x>=1280 || y<0 || y>=720)return;
    p+=y*stride+3*x;p[0]=(uint8_t)b;p[1]=(uint8_t)g;p[2]=(uint8_t)r;
}
static void line(uint8_t *p,uint32_t stride,int x0,int y0,int x1,int y1,int r,int g,int b)
{
    int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy;
    for(;;) {
        for(int yy=-1;yy<=1;yy++)for(int xx=-1;xx<=1;xx++)point(p,stride,x0+xx,y0+yy,r,g,b);
        if(x0==x1 && y0==y1)break;
        int e=2*err;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}
    }
}
static void fill_rect(uint8_t *p,uint32_t stride,int x0,int y0,int x1,int y1,int r,int g,int b)
{
    if(x0<0)x0=0;
    if(y0<0)y0=0;
    if(x1>=1280)x1=1279;
    if(y1>=720)y1=719;
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++)point(p,stride,x,y,r,g,b);
}
static void hex_glyph(uint8_t *p,uint32_t stride,int x,int y,unsigned digit,int r,int g,int b)
{
    static const uint8_t glyph[16][5]={
        {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},
        {5,5,7,1,1},{7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},
        {7,5,7,5,7},{7,5,7,1,7},{7,5,7,5,5},{6,5,6,5,6},
        {7,4,4,4,7},{6,5,5,5,6},{7,4,7,4,7},{7,4,7,4,4}
    };
    if(digit>15u)return;
    for(int row=0;row<5;row++)for(int col=0;col<3;col++)
        if(glyph[digit][row]&(1u<<(2-col)))
            fill_rect(p,stride,x+2*col,y+2*row,x+2*col+1,y+2*row+1,r,g,b);
}
static void object_tag(uint8_t *p,uint32_t stride,const LiveObject *o)
{
    unsigned color=LIVE_TAG_COLOR(o->match_error),shape=LIVE_TAG_SHAPE(o->match_error);
    int x=(int)o->bbox[0]+2,y=(int)o->bbox[1]+2;
    if(y>704)y=(int)o->bbox[3]-13;
    if(y<1)y=1;
    if(x>1256)x=1256;
    fill_rect(p,stride,x-1,y-1,x+14,y+11,0,0,0);
    hex_glyph(p,stride,x,y,color,255,255,255);
    hex_glyph(p,stride,x+8,y,shape,LIVE_TAG_VALID(o->match_error)?0:255,
              LIVE_TAG_VALID(o->match_error)?255:0,0);
}
void LiveDraw(uint8_t *p,uint32_t stride,const LiveResult *r)
{
    if(!p || !r || r->valid!=1 || stride<1280*3 || r->count>8)return;
    /* Validate the complete result before writing any pixel. */
    for(int i=0;i<4;i++)if(r->roi[2*i]>=1280 || r->roi[2*i+1]>=720)return;
    for(uint32_t i=0;i<r->count;i++) {
        const LiveObject *o=&r->objects[i];
        if(o->bbox[0]>o->bbox[2] || o->bbox[1]>o->bbox[3] ||
           o->bbox[2]>=1280 || o->bbox[3]>=720 || o->center[0]>=1280 || o->center[1]>=720)return;
    }
    for(int i=0;i<4;i++) {int j=(i+1)%4;line(p,stride,r->roi[2*i],r->roi[2*i+1],r->roi[2*j],r->roi[2*j+1],255,255,0);}
    for(uint32_t i=0;i<r->count;i++) {
        const LiveObject *o=&r->objects[i];int x0=o->bbox[0],y0=o->bbox[1],x1=o->bbox[2],y1=o->bbox[3];
        line(p,stride,x0,y0,x1,y0,0,255,0);line(p,stride,x1,y0,x1,y1,0,255,0);
        line(p,stride,x1,y1,x0,y1,0,255,0);line(p,stride,x0,y1,x0,y0,0,255,0);
        line(p,stride,o->center[0]-4,o->center[1],o->center[0]+4,o->center[1],255,0,255);
        line(p,stride,o->center[0],o->center[1]-4,o->center[0],o->center[1]+4,255,0,255);
        object_tag(p,stride,o);
    }
}


/* Limit displayed-edge lag to four pixels. A match may absorb up to
 * eight pixels of extent jitter only when its center stays within four pixels.
 * Larger translation, missing objects, ambiguity and expiry update immediately. */
static int box_near(const uint32_t *a,const uint32_t *b,unsigned count,unsigned limit)
{
    for(unsigned i=0;i<count;i++) {
        uint32_t distance=a[i]>b[i]?a[i]-b[i]:b[i]-a[i];
        if(distance>limit)return 0;
    }
    return 1;
}
static int box_match(const LiveObject *a,const LiveObject *b)
{
    uint32_t ac[2]={a->bbox[0]+a->bbox[2],a->bbox[1]+a->bbox[3]};
    uint32_t bc[2]={b->bbox[0]+b->bbox[2],b->bbox[1]+b->bbox[3]};
    return box_near(a->bbox,b->bbox,4,8) && box_near(ac,bc,2,8);
}
void LiveStabilize(const LiveResult *raw,LiveResult *shown,
                   LiveBoxState *state,uint64_t now_ms)
{
    if(!shown)return;
    if(!raw || !state || raw->valid!=1 || raw->count>8) {
        memset(shown,0,sizeof(*shown));
        if(state)memset(state,0,sizeof(*state));
        return;
    }
    LiveResult next=*raw;
    const LiveResult *previous=&state->shown;
    int reuse=previous->valid==1 && previous->count<=8 && now_ms>=state->stamp_ms &&
              now_ms-state->stamp_ms<=300 && box_near(raw->roi,previous->roi,8,4);
    if(reuse) {
        unsigned matches[8]={0},degree[8]={0},selected[8]={0};
        /* A near pair is usable only when both sides have exactly one near
         * partner. Ambiguity updates immediately from this frame's raw box. */
        for(unsigned i=0;i<raw->count;i++)for(unsigned j=0;j<previous->count;j++)
            if(box_match(&raw->objects[i],&previous->objects[j])) {
                ++matches[i];++degree[j];selected[i]=j;
            }
        for(unsigned i=0;i<raw->count;i++) {
            unsigned j=selected[i];
            if(matches[i]!=1 || degree[j]!=1)continue;
            LiveObject *o=&next.objects[i];
            for(unsigned edge=0;edge<4;edge++) {
                uint32_t current=o->bbox[edge],old=previous->objects[j].bbox[edge];
                uint32_t delta=current>old?current-old:old-current;
                o->bbox[edge]=delta<=4?old:(current>old?current-4:current+4);
            }
            o->center[0]=(o->bbox[0]+o->bbox[2])/2;
            o->center[1]=(o->bbox[1]+o->bbox[3])/2;
        }
    }
    state->shown=next;state->stamp_ms=now_ms;*shown=next;
}
