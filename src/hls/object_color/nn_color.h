/* Integer OpenCV uint8 HSV and configured color votes. RGB32: low R, then G,B.
 * Masks are nonzero=foreground, without contour filling or morphology.
 * Caller owns a stable image/mask for the complete call. No allocation/floats. */
#ifndef NN_COLOR_H
#define NN_COLOR_H
#include <stdint.h>
#include "nn_color_tables.h"

typedef struct { uint32_t center0,center1,tolerance,min_saturation; } nn_color_rule;
/* IDs: 0 unknown; 1..5 follow the ordered rules. Confidence is eligible votes,
 * not a probability of correctness. Output remains untouched on bad input. */
typedef struct { uint32_t label,confidence16,eligible,counts[5]; } nn_color_result;

static void nn_rgb32_hsv(uint32_t rgb,uint8_t hsv[3]) {
    int32_t r=rgb&255U,g=(rgb>>8)&255U,b=(rgb>>16)&255U;
    int32_t v=r>g?r:g;v=v>b?v:b;
    int32_t low=r<g?r:g;low=low<b?low:b;
    int32_t diff=v-low,h=0;
    uint32_t saturation=((uint32_t)diff*NN_SATURATION_DIV[v]+2048U)/4096U;
    if(diff) {
        int32_t raw=v==r?g-b:v==g?b-r+2*diff:r-g+4*diff;
        int32_t scaled=raw*(int32_t)NN_HUE_DIV[diff]+2048;
        h=scaled>=0?scaled/4096:-((-scaled+4095)/4096);
        if(h<0) h+=180;
    }
    hsv[0]=(uint8_t)h;hsv[1]=(uint8_t)saturation;hsv[2]=(uint8_t)v;
}

static uint32_t nn_hue_distance(uint32_t h,uint32_t center) {
    uint32_t d=h>center?h-center:center-h;
    return d<180U-d?d:180U-d;
}

static uint32_t nn_color_rules_valid(const nn_color_rule rules[5]) {
    if(!rules) return 0;
    for(uint32_t i=0;i<5;++i)
        if(rules[i].center0>=180 || rules[i].center1>180 || rules[i].tolerance>90 || rules[i].min_saturation>255) return 0;
    return 1;
}

static uint32_t nn_color_vote(const uint32_t *rgb,uint32_t rgb_stride_words,
        const uint8_t *mask,uint32_t mask_stride,uint32_t width,uint32_t height,
        const nn_color_rule rules[5],nn_color_result *output) {
    if(!rgb || !mask || !rules || !output || !width || width>2048 || !height || height>2048 ||
       rgb_stride_words<width || rgb_stride_words>16384 || mask_stride<width || mask_stride>65536) return 1;
    if(!nn_color_rules_valid(rules)) return 1;
    nn_color_result result={0,0,0,{0,0,0,0,0}};
    for(uint32_t y=0;y<height;++y) for(uint32_t x=0;x<width;++x) {
        if(!mask[y*mask_stride+x]) continue;
        uint8_t hsv[3];nn_rgb32_hsv(rgb[y*rgb_stride_words+x],hsv);
        if(hsv[2]<20) continue;
        uint32_t best=181,owner=0;
        for(uint32_t i=0;i<5;++i) {
            uint32_t d=nn_hue_distance(hsv[0],rules[i].center0);
            if(rules[i].center1<180) {
                uint32_t d2=nn_hue_distance(hsv[0],rules[i].center1);if(d2<d) d=d2;
            }
            /* Strict improvement preserves configured order for pixel ties. */
            if(d<best && d<=rules[i].tolerance && hsv[1]>=rules[i].min_saturation) {best=d;owner=i;}
        }
        if(best<=180) {++result.counts[owner];++result.eligible;}
    }
    if(result.eligible) {
        uint32_t winner=0,tied=0;
        for(uint32_t i=1;i<5;++i) if(result.counts[i]>result.counts[winner]) winner=i;
        for(uint32_t i=0;i<5;++i) if(result.counts[i]==result.counts[winner]) ++tied;
        if(tied==1 && 2U*result.counts[winner]>=result.eligible) result.label=winner+1;
        result.confidence16=(uint32_t)(((uint64_t)result.counts[winner]*65535U+result.eligible/2U)/result.eligible);
    }
    *output=result;return 0;
}
#endif
