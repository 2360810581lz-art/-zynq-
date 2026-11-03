#include "vision_black_border.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIDE 1920u
#define PATCH 85
typedef struct { uint32_t x,y,w,h,area,seed; double score,quad[8]; } Component;
typedef struct { double slope,offset,support,residual; } Line;

void VisionBlackBorder_Defaults(VisionBlackBorderSettings *s)
{
    if (!s) return;
    s->dark_threshold=110; s->close_kernel=9; s->interior_threshold=150;
    s->interior_close_kernel=21; s->interior_dilation_kernel=5;
    s->corner_radius=35; s->bottom_inset=5; s->min_confidence=0.65;
}

uint32_t VisionBlackBorder_ScratchBytes(uint32_t w,uint32_t h)
{
    if (w<64 || h<64 || w>MAX_SIDE || h>1080) return 0;
    /* Four byte masks followed by an aligned flood queue. The queue storage
     * is reused by line fitting and the bounded corner response patch. */
    return 8*w*h+8+4*PATCH*PATCH+MAX_SIDE*41;
}

static int overlap(const void *a,uint64_t na,const void *b,uint64_t nb)
{
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    if (na>UINTPTR_MAX-x || nb>UINTPTR_MAX-y) return 1;
    return x<y+nb && y<x+na;
}

/* Rectangular binary morphology, with OpenCV's neutral constant border.
 * Running sums make cost independent of kernel size. */
static void morph(uint8_t *a,uint8_t *b,uint32_t w,uint32_t h,uint32_t k,int dilate)
{
    int r=(int)(k/2),x,y,t,sum,needed;
    for (y=0;y<(int)h;y++) {
        sum=0;
        for (t=0;t<=r;t++) sum+=a[y*w+t]!=0;
        for (x=0;x<(int)w;x++) {
            needed=(x+r<(int)w?x+r:(int)w-1)-(x-r>0?x-r:0)+1;
            b[y*w+x]=(uint8_t)(dilate?sum!=0:sum==needed);
            if (x-r>=0) sum-=a[y*w+x-r]!=0;
            if (x+r+1<(int)w) sum+=a[y*w+x+r+1]!=0;
        }
    }
    for (x=0;x<(int)w;x++) {
        sum=0;
        for (t=0;t<=r;t++) sum+=b[t*w+x]!=0;
        for (y=0;y<(int)h;y++) {
            needed=(y+r<(int)h?y+r:(int)h-1)-(y-r>0?y-r:0)+1;
            a[y*w+x]=(uint8_t)(dilate?sum!=0:sum==needed);
            if (y-r>=0) sum-=b[(y-r)*w+x]!=0;
            if (y+r+1<(int)h) sum+=b[(y+r+1)*w+x]!=0;
        }
    }
}

static Component flood(uint8_t *mask,uint32_t *queue,uint32_t w,uint32_t h,uint32_t seed,uint8_t mark)
{
    Component c; uint32_t head=0,tail=1,xmax=0,ymax=0,i,x,y;
    int dx,dy,sums[4]={2147483647,-2147483647,-2147483647,2147483647};
    memset(&c,0,sizeof(c)); c.x=w; c.y=h; c.seed=seed;
    queue[0]=seed; mask[seed]=mark;
    while (head<tail) {
        i=queue[head++]; x=i%w; y=i/w;
        if (x<c.x) c.x=x;
        if (y<c.y) c.y=y;
        if (x>xmax) xmax=x;
        if (y>ymax) ymax=y;
        if ((int)(x+y)<sums[0]) { sums[0]=(int)(x+y); c.quad[0]=x; c.quad[1]=y; }
        if ((int)x-(int)y>sums[1]) { sums[1]=(int)x-(int)y; c.quad[2]=x; c.quad[3]=y; }
        if ((int)(x+y)>sums[2]) { sums[2]=(int)(x+y); c.quad[4]=x; c.quad[5]=y; }
        if ((int)x-(int)y<sums[3]) { sums[3]=(int)x-(int)y; c.quad[6]=x; c.quad[7]=y; }
        for (dy=-1;dy<=1;dy++) for (dx=-1;dx<=1;dx++) {
            uint32_t nx=(uint32_t)((int)x+dx),ny=(uint32_t)((int)y+dy),n;
            if (nx>=w || ny>=h) continue;
            n=ny*w+nx;
            if (mask[n]==1) { mask[n]=mark; queue[tail++]=n; }
        }
    }
    c.area=tail; c.w=xmax-c.x+1; c.h=ymax-c.y+1; return c;
}

static int candidate(Component *c,uint32_t w,uint32_t h,int interior)
{
    double area=(double)w*h,aspect=(double)c->w/c->h,dist;
    if (c->w<0.30*w || c->w>0.85*w || c->h<0.45*h || c->h>0.96*h ||
        c->area<(interior?0.12:0.005)*area || c->area>(interior?0.70:0.15)*area ||
        aspect<0.60 || aspect>1.55) return 0;
    if (interior && (c->x<=1 || c->y<=1 || c->x+c->w>=w-1 || c->y+c->h>=h-1)) return 0;
    dist=fabs((c->x+c->w*0.5)/w-0.5)+fabs((c->y+c->h*0.5)/h-0.5);
    c->score=c->area*fmax(0.25,1-0.5*dist); return 1;
}

static int select_component(uint8_t *mask,uint32_t *queue,uint32_t w,uint32_t h,int interior,Component *best)
{
    uint32_t i,n=w*h; Component c; memset(best,0,sizeof(*best));
    for (i=0;i<n;i++) if (mask[i]==1) {
        c=flood(mask,queue,w,h,i,2);
        if (candidate(&c,w,h,interior) && c.score>best->score) *best=c;
    }
    if (!best->area) return -1;
    for (i=0;i<n;i++) if (mask[i]==2) mask[i]=1;
    c=flood(mask,queue,w,h,best->seed,2);
    for (i=0;i<n;i++) mask[i]=(uint8_t)(mask[i]==2);
    return 0;
}

static int compare_double(const void *a,const void *b)
{
    double x=*(const double *)a,y=*(const double *)b; return (x>y)-(x<y);
}
static double median(double *values,uint32_t n)
{
    qsort(values,n,sizeof(*values),compare_double);
    return n%2?values[n/2]:(values[n/2-1]+values[n/2])*0.5;
}

static int edge(const uint8_t *mask,uint32_t w,const Component *c,int side,void *work,Line *line)
{
    double *a=work,*b=a+MAX_SIDE,*res=b+MAX_SIDE,*temp=res+MAX_SIDE;
    uint8_t *keep=(uint8_t *)(temp+MAX_SIDE);
    uint32_t start,stop,p,q,n=0,count,iter; double mid,mad,cut;
    if (side>=2) { start=c->x+(uint32_t)nearbyint(c->w*0.08); stop=c->x+c->w-(uint32_t)nearbyint(c->w*0.08); }
    else { start=c->y+(uint32_t)nearbyint(c->h*0.08); stop=c->y+c->h-(uint32_t)nearbyint(c->h*0.08); }
    for (p=start;p<stop;p++) {
        int found=-1;
        if (side==0) {
            for (q=c->x;q<c->x+c->w && q<c->x+0.22*c->w;q++) {
                if (mask[p*w+q]) found=(int)q;
                else if (found>=0) break;
            }
        } else if (side==1) {
            for (q=c->x+c->w;q-->c->x && q>c->x+0.78*c->w;) {
                if (mask[p*w+q]) found=(int)q;
                else if (found>=0) break;
            }
        } else if (side==3) {
            for (q=c->y;q<c->y+c->h && q<c->y+0.22*c->h;q++) {
                if (mask[q*w+p]) found=(int)q;
                else if (found>=0) break;
            }
        } else {
            for (q=c->y+c->h;q-->c->y && q>c->y+0.78*c->h;) {
                if (mask[q*w+p]) found=(int)q;
                else if (found>=0) break;
            }
        }
        if (found>=0) { a[n]=p; b[n]=found; keep[n++]=1; }
    }
    if (n<20) return -1;
    for (iter=0;iter<6;iter++) {
        double sx=0,sy=0,sxx=0,sxy=0,den; count=0;
        for (p=0;p<n;p++) if (keep[p]) { sx+=a[p]; sy+=b[p]; sxx+=a[p]*a[p]; sxy+=a[p]*b[p]; count++; }
        if (count<20) break;
        den=count*sxx-sx*sx; if (den<=0) return -1;
        line->slope=(count*sxy-sx*sy)/den; line->offset=(sy-line->slope*sx)/count;
        count=0;
        for (p=0;p<n;p++) { res[p]=fabs(b[p]-(line->slope*a[p]+line->offset)); if (keep[p]) temp[count++]=res[p]; }
        mid=median(temp,count); count=0;
        for (p=0;p<n;p++) if (keep[p]) temp[count++]=fabs(res[p]-mid);
        mad=median(temp,count); cut=fmax(2.5,mid+2.5*fmax(mad,1));
        for (p=0;p<n;p++) keep[p]=(uint8_t)(res[p]<=cut);
    }
    count=0;
    for (p=0;p<n;p++) if (keep[p]) temp[count++]=fabs(b[p]-(line->slope*a[p]+line->offset));
    if (count<20) return -1;
    line->support=(double)count/(stop-start); line->residual=median(temp,count); return 0;
}

/* Local Shi-Tomasi: Sobel3 gradients, 11x11 covariance and 3x3 maxima.
 * A fixed patch keeps stack/DDR usage bounded, independent of object count. */
static void covariance(const uint8_t *mask,uint32_t w,uint32_t h,int x,int y,int *out)
{
    int gx=0,gy=0;
    if (x>0 && y>0 && x<(int)w-1 && y<(int)h-1) {
        int p=y*(int)w+x;
        gx=mask[p-w+1]+2*mask[p+1]+mask[p+w+1]-mask[p-w-1]-2*mask[p-1]-mask[p+w-1];
        gy=mask[p+w-1]+2*mask[p+w]+mask[p+w+1]-mask[p-w-1]-2*mask[p-w]-mask[p-w+1];
    }
    out[0]=gx*gx; out[1]=gx*gy; out[2]=gy*gy;
}

static void snap(const uint8_t *mask,uint32_t w,uint32_t h,double *point,double radius,void *work)
{
    float *response=work;
    int (*columns)[3]=(int (*)[3])(response+PATCH*PATCH);
    int cx=(int)point[0],cy=(int)point[1],x,y,dx,dy,c,j,term[3],sums[3];
    double best=radius*radius+1,maxval=0; int bx=cx,by=cy;
    const int half=PATCH/2,extended=PATCH+10;
    /* Binary Sobel products and their rolling sums are exact integers. Reuse
     * the existing workspace after the response patch; no extra allocation. */
    memset(columns,0,(size_t)extended*3*sizeof(int));
    for (dy=-5;dy<=5;dy++) for (c=0;c<extended;c++) {
        covariance(mask,w,h,cx-half+c-5,cy-half+dy,term);
        for(j=0;j<3;j++) columns[c][j]+=term[j];
    }
    for (y=0;y<PATCH;y++) {
        sums[0]=sums[1]=sums[2]=0;
        for(c=0;c<11;c++) for(j=0;j<3;j++) sums[j]+=columns[c][j];
        for (x=0;x<PATCH;x++) {
            int px=cx+x-half,py=cy+y-half;
            if (px<6 || py<6 || px>=(int)w-6 || py>=(int)h-6) response[y*PATCH+x]=0;
            else {
                double xx=sums[0],xy=sums[1],yy=sums[2];
                double value=(xx+yy-sqrt((xx-yy)*(xx-yy)+4*xy*xy))*0.5;
                response[y*PATCH+x]=(float)value; if(value>maxval) maxval=value;
            }
            if(x+1<PATCH) for(j=0;j<3;j++) sums[j]+=columns[x+11][j]-columns[x][j];
        }
        if(y+1<PATCH) for(c=0;c<extended;c++) {
            covariance(mask,w,h,cx-half+c-5,cy-half+y-5,term);
            for(j=0;j<3;j++) columns[c][j]-=term[j];
            covariance(mask,w,h,cx-half+c-5,cy-half+y+6,term);
            for(j=0;j<3;j++) columns[c][j]+=term[j];
        }
    }
    for (y=1;y<PATCH-1;y++) for (x=1;x<PATCH-1;x++) {
        double value=response[y*PATCH+x],distance=(x-half)*(x-half)+(y-half)*(y-half); int peak=1;
        if (distance>radius*radius || value<=0.01*maxval || distance>=best) continue;
        for (dy=-1;dy<=1;dy++) for (dx=-1;dx<=1;dx++) if (response[(y+dy)*PATCH+x+dx]>value) peak=0;
        if (peak) { best=distance; bx=cx+x-half; by=cy+y-half; }
    }
    point[0]=bx; point[1]=by;
}

static int intersect(Line x,Line y,double *point)
{
    double den=1-y.slope*x.slope;
    if (fabs(den)<1e-6) return -1;
    point[1]=(y.slope*x.offset+y.offset)/den; point[0]=x.slope*point[1]+x.offset; return 0;
}

/* Measure all four tape edges. Raw low-threshold pixels near the measured
 * segment retain disconnected tape pieces omitted by component selection. */
static int measured_quad(const uint8_t *component,const uint8_t *gray,uint32_t threshold,
    uint32_t *queue,uint32_t w,uint32_t h,const Component *dark,double *quad)
{
    Line lines[4]; uint32_t i,j; int radius=(int)((w+159u)/160u);
    for(i=0;i<4;i++) {
        if(edge(component,w,dark,(int)i,queue,&lines[i]) || lines[i].support<0.75 ||
           lines[i].residual>2.5 || fabs(lines[i].slope)>0.35)return -1;
    }
    if(intersect(lines[0],lines[3],quad) || intersect(lines[1],lines[3],quad+2) ||
       intersect(lines[1],lines[2],quad+4) || intersect(lines[0],lines[2],quad+6))return -1;
    for(i=0;i<4;i++)if(!isfinite(quad[2*i]) || !isfinite(quad[2*i+1]) ||
        quad[2*i]<2 || quad[2*i]>=w-2 || quad[2*i+1]<2 || quad[2*i+1]>=h-2)return -1;
    for(i=0;i<4;i++) {
        uint32_t next=(i+1)%4,run=0,longest=0,hits=0;
        double x0=quad[2*i],y0=quad[2*i+1],dx=quad[2*next]-x0,dy=quad[2*next+1]-y0;
        uint32_t steps=(uint32_t)ceil(fmax(fabs(dx),fabs(dy)));
        if(steps<20 || steps>MAX_SIDE)return -1;
        for(j=0;j<=steps;j++) {
            int x=(int)nearbyint(x0+dx*j/steps),y=(int)nearbyint(y0+dy*j/steps),u,v,found=0;
            for(v=-radius;v<=radius && !found;v++)for(u=-radius;u<=radius;u++) {
                int a=x+u,b=y+v;
                if(a>=0 && a<(int)w && b>=0 && b<(int)h && gray[b*w+a]<threshold){found=1;break;}
            }
            if(found){hits++;run=0;}else{run++;if(run>longest)longest=run;}
        }
        if(hits<0.75*(steps+1) || longest>0.12*steps)return -1;
    }
    return 0;
}

/* Repair short gaps only when four measured finite segments have evidence.
 * Reuse the existing masks and fitting workspace; never infer a missing side. */
static int closed_interior(const uint8_t *component,const uint8_t *gray,uint32_t threshold,
    uint8_t *mask,uint32_t *queue,uint32_t w,uint32_t h,const Component *dark,Component *white)
{
    double quad[8];uint32_t i,j,n=w*h;
    if(measured_quad(component,gray,threshold,queue,w,h,dark,quad))return -1;
    memcpy(mask,component,n);
    for(i=0;i<4;i++) {
        uint32_t next=(i+1)%4;
        double x0=quad[2*i],y0=quad[2*i+1],dx=quad[2*next]-x0,dy=quad[2*next+1]-y0;
        uint32_t steps=(uint32_t)ceil(fmax(fabs(dx),fabs(dy)));
        for(j=0;j<=steps;j++) {
            int x=(int)nearbyint(x0+dx*j/steps),y=(int)nearbyint(y0+dy*j/steps),u,v;
            for(v=-1;v<=1;v++)for(u=-1;u<=1;u++)mask[(y+v)*w+x+u]=1;
        }
    }
    for(i=0;i<n;i++)mask[i]=(uint8_t)!mask[i];
    return select_component(mask,queue,w,h,1,white);
}

/* Failure-only recovery for a border split into separate dark components.
 * Expand only unsupported sides of the measured component, by at most 10%.
 * A modest brighter threshold can observe reflective tape; all four lines and
 * their finite segments must still pass the existing closed_interior guards. */
static int disconnected_interior(const uint8_t *gray,uint8_t *component,uint8_t *mask,
    uint8_t *temp,uint32_t *queue,uint32_t w,uint32_t h,
    const VisionBlackBorderSettings *s,Component *dark,Component *white)
{
    Component search=*dark; Line line; uint32_t i,n=w*h,x0=dark->x,y0=dark->y;
    uint32_t x1=x0+dark->w,y1=y0+dark->h;
    uint32_t dx=(dark->w+9)/10,dy=(dark->h+9)/10;
    for(i=0;i<4;i++) {
        if(!edge(component,w,dark,(int)i,queue,&line) && line.support>=0.75 &&
            line.residual<=2.5 && fabs(line.slope)<=0.35) continue;
        if(i==0)x0=x0>dx?x0-dx:0;
        else if(i==1)x1=x1+dx<w?x1+dx:w;
        else if(i==2)y1=y1+dy<h?y1+dy:h;
        else y0=y0>dy?y0-dy:0;
    }
    search.x=x0;search.y=y0;search.w=x1-x0;search.h=y1-y0;
    for(i=0;i<n;i++)component[i]=(uint8_t)(gray[i]<s->dark_threshold+30u);
    morph(component,temp,w,h,s->close_kernel,1);
    morph(component,temp,w,h,s->close_kernel,0);
    if(closed_interior(component,gray,s->dark_threshold+30u,mask,queue,w,h,&search,white))return -1;
    if(white->x<search.x || white->y<search.y ||
        white->x+white->w>x1 || white->y+white->h>y1)return -1;
    *dark=search;return 0;
}

int VisionBlackBorder_Detect(const VisionFrameView *f,const VisionBlackBorderSettings *s,
    void *scratch,uint32_t capacity,VisionBlackBorderResult *out)
{
    VisionBlackBorderResult r; Component dark,white; Line lines[3];
    uint32_t need,n,i,x,y,threshold; uint8_t *gray,*mask,*temp,*component; uint32_t *queue;
    double center[2]={0,0},normal[2],length,area=0,minimum=1,maximum=0,cross;
    int failure=0,recovered=0;
    if (!out) return -1;
    memset(out,0,sizeof(*out)); memset(&r,0,sizeof(r));
    if (!f || !s || !scratch || !f->bgr || !f->generation) return -1;
    need=VisionBlackBorder_ScratchBytes(f->width,f->height);
    if (!need || capacity<need || ((uintptr_t)scratch&7) || f->stride<3*f->width ||
        (uint64_t)(f->height-1)*f->stride+3*f->width>f->bytes) return -1;
    if (overlap(scratch,need,f->bgr,f->bytes) || overlap(scratch,need,out,sizeof(*out)) ||
        overlap(scratch,need,s,sizeof(*s)) || overlap(out,sizeof(*out),f->bgr,f->bytes) ||
        overlap(out,sizeof(*out),s,sizeof(*s)) || overlap(scratch,need,f,sizeof(*f))) return -1;
    if (s->dark_threshold<1 || s->dark_threshold>254 || s->interior_threshold<1 || s->interior_threshold>224 ||
        !s->close_kernel || s->close_kernel>31 || !(s->close_kernel&1) ||
        !s->interior_close_kernel || s->interior_close_kernel>31 || !(s->interior_close_kernel&1) ||
        !s->interior_dilation_kernel || s->interior_dilation_kernel>31 || !(s->interior_dilation_kernel&1) ||
        !isfinite(s->corner_radius) || s->corner_radius<0 || s->corner_radius>35 ||
        !isfinite(s->bottom_inset) || s->bottom_inset<0 || s->bottom_inset>20 ||
        !isfinite(s->min_confidence) || s->min_confidence<0 || s->min_confidence>1) return -1;
    n=f->width*f->height; gray=scratch; mask=gray+n; temp=mask+n; component=temp+n;
    queue=(uint32_t *)(((uintptr_t)(component+n)+7)&~(uintptr_t)7);
    for (y=0;y<f->height;y++) for (x=0;x<f->width;x++) {
        const uint8_t *b=f->bgr+y*f->stride+3*x;
        gray[y*f->width+x]=(uint8_t)((3735u*b[0]+19235u*b[1]+9798u*b[2]+16384u)>>15);
    }
    for (i=0;i<n;i++) mask[i]=(uint8_t)(gray[i]<s->dark_threshold);
    morph(mask,temp,f->width,f->height,s->close_kernel,1);
    morph(mask,temp,f->width,f->height,s->close_kernel,0);
    if (select_component(mask,queue,f->width,f->height,0,&dark)) return -2;
    memcpy(component,mask,n);
    for (threshold=s->interior_threshold;threshold<=s->interior_threshold+30;threshold+=10) {
        for (i=0;i<n;i++) mask[i]=(uint8_t)(gray[i]<threshold);
        morph(mask,temp,f->width,f->height,s->interior_close_kernel,1);
        morph(mask,temp,f->width,f->height,s->interior_close_kernel,0);
        morph(mask,temp,f->width,f->height,s->interior_dilation_kernel,1);
        for (i=0;i<n;i++) mask[i]=(uint8_t)!mask[i];
        if (!select_component(mask,queue,f->width,f->height,1,&white)) break;
    }
    if (threshold>s->interior_threshold+30 &&
        closed_interior(component,gray,s->dark_threshold,mask,queue,f->width,f->height,&dark,&white)) { failure=-3; goto recover; }
located:
    memset(&r,0,sizeof(r));
    center[0]=center[1]=area=maximum=0;minimum=1;
    memcpy(r.corners,white.quad,sizeof(r.corners));
    snap(component,f->width,f->height,r.corners,s->corner_radius,queue);
    snap(component,f->width,f->height,r.corners+2,s->corner_radius,queue);
    for (i=0;i<3;i++) {
        if (edge(component,f->width,&dark,(int)i,queue,&lines[i])) { failure=-4; goto recover; }
        r.support[i]=lines[i].support; r.residual[i]=lines[i].residual;
        minimum=fmin(minimum,r.support[i]); maximum=fmax(maximum,r.residual[i]);
    }
    /* White-component extrema can stop at an object shadow. Keep the
     * established inner corners when they agree with measured tape lines;
     * a substantial mismatch must use a fully supported four-edge repair. */
    {
        Line top; double quad[8],tolerance=0.02*f->width;
        if(!edge(component,f->width,&dark,3,queue,&top) && top.support>=0.75 &&
           top.residual<=2.5 && fabs(top.slope)<=0.35 &&
           (fabs(r.corners[0]-(lines[0].slope*r.corners[1]+lines[0].offset))>tolerance ||
            fabs(r.corners[2]-(lines[1].slope*r.corners[3]+lines[1].offset))>tolerance ||
            fabs(r.corners[1]-(top.slope*r.corners[0]+top.offset))>tolerance ||
            fabs(r.corners[3]-(top.slope*r.corners[2]+top.offset))>tolerance)) {
            if(measured_quad(component,gray,s->dark_threshold+(recovered?30u:0u),queue,
                f->width,f->height,&dark,quad)){failure=-4;goto recover;}
            memcpy(r.corners,quad,sizeof(r.corners));
            snap(component,f->width,f->height,r.corners,s->corner_radius,queue);
            snap(component,f->width,f->height,r.corners+2,s->corner_radius,queue);
        }
    }
    if (intersect(lines[1],lines[2],r.corners+4) || intersect(lines[0],lines[2],r.corners+6)) { failure=-4; goto recover; }
    for (i=0;i<4;i++) { center[0]+=white.quad[2*i]*0.25; center[1]+=white.quad[2*i+1]*0.25; }
    normal[0]=center[0]-(r.corners[4]+r.corners[6])*0.5;
    normal[1]=center[1]-(r.corners[5]+r.corners[7])*0.5;
    length=hypot(normal[0],normal[1]);
    if (length>0) for (i=4;i<8;i++) r.corners[i]+=normal[i%2]*s->bottom_inset/length;
    for (i=0;i<4;i++) {
        uint32_t j=(i+1)%4,k=(i+2)%4;
        x=2*i; y=2*j;
        if (!isfinite(r.corners[x]) || !isfinite(r.corners[x+1]) || r.corners[x]<0 || r.corners[x]>=f->width ||
            r.corners[x+1]<0 || r.corners[x+1]>=f->height) { failure=-5; goto recover; }
        cross=(r.corners[y]-r.corners[x])*(r.corners[2*k+1]-r.corners[y+1])-
              (r.corners[y+1]-r.corners[x+1])*(r.corners[2*k]-r.corners[y]);
        if (!(cross>0)) { failure=-5; goto recover; }
        area+=r.corners[x]*r.corners[y+1]-r.corners[y]*r.corners[x+1];
    }
    r.area_fraction=fabs(area)*0.5/n;
    if (r.area_fraction<0.20 || r.area_fraction>0.65) { failure=-5; goto recover; }
    r.confidence=fmin(1,0.55+0.35*minimum+0.10*fmax(0,1-maximum/5));
    if (r.confidence<s->min_confidence) { failure=-6; goto recover; }
    r.bbox[0]=dark.x; r.bbox[1]=dark.y; r.bbox[2]=dark.w; r.bbox[3]=dark.h;
    r.generation=f->generation; *out=r; return 0;
recover:
    if(recovered || s->dark_threshold>224u ||
        disconnected_interior(gray,component,mask,temp,queue,f->width,f->height,s,&dark,&white))
        return failure;
    recovered=1;goto located;
}
