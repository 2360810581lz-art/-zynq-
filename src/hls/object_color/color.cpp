#include "color.h"
#include "nn_color.h"
namespace {
typedef unsigned PixelCandidate;
PixelCandidate closer(PixelCandidate left,PixelCandidate right) {
#pragma HLS INLINE
    // Distance is high bits; ascending owner IDs preserve rule-order ties.
    return left<=right ? left : right;
}
ap_uint<3> pixel_vote(ap_uint<24> bgr) {
#pragma HLS INLINE off
#pragma HLS PIPELINE II=1
    uint32_t rgb=(bgr.to_uint()&0xff00U)|((bgr.to_uint()&255U)<<16)|(bgr.to_uint()>>16);
    uint8_t hsv[3];nn_rgb32_hsv(rgb,hsv);
    const int centers[6]={7,179,13,28,60,110};
    const int owners[6]={1,1,2,3,4,5};
    const int tolerances[6]={8,8,7,12,25,20};
    const int saturation[6]={80,80,80,50,20,20};
    PixelCandidate candidates[6];
#pragma HLS ARRAY_PARTITION variable=candidates complete
    for(int i=0;i<6;i++) {
#pragma HLS UNROLL
        int d=nn_hue_distance(hsv[0],centers[i]);
        bool eligible=hsv[2]>=20 && hsv[1]>=saturation[i] && d<=tolerances[i];
        PixelCandidate candidate=(eligible?unsigned(d):181U)*8U+unsigned(owners[i]);
        candidates[i]=candidate;
    }
    PixelCandidate first=closer(candidates[0],candidates[1]);
    PixelCandidate second=closer(candidates[2],candidates[3]);
    PixelCandidate third=closer(candidates[4],candidates[5]);
    PixelCandidate best=closer(closer(first,second),third);
    return best/8U==181U ? ap_uint<3>(0) : ap_uint<3>(best&7U);
}
}
void live_object_color(hls::stream<color_members>& membership,
    hls::stream<color_pixel>& pixels,hls::stream<color_result>& results,
    ap_uint<16> width,ap_uint<16> height,ap_uint<8> part_count,
    ap_uint<4> object_count,ap_uint<4>& status) {
#pragma HLS INTERFACE axis port=membership
#pragma HLS INTERFACE axis port=pixels
#pragma HLS INTERFACE axis port=results
#pragma HLS INTERFACE s_axilite port=width bundle=control
#pragma HLS INTERFACE s_axilite port=height bundle=control
#pragma HLS INTERFACE s_axilite port=part_count bundle=control
#pragma HLS INTERFACE s_axilite port=object_count bundle=control
#pragma HLS INTERFACE s_axilite port=status bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control
    unsigned w=width,h=height,n=part_count,count=object_count;
    ap_uint<4> faults=0;
    ap_uint<4> owner[64];ap_uint<18> votes[40],sampled[8];
#pragma HLS RESOURCE variable=owner core=RAM_2P_LUTRAM
#pragma HLS RESOURCE variable=votes core=RAM_2P_LUTRAM
    if(!w || w>640 || !h || h>360 || n>64 || count>8 || count>n)faults=1;
    else {
        for(int k=0;k<64;k++)owner[k]=0;
        for(int i=0;i<8;i++) {
            sampled[i]=0;
            for(int c=0;c<5;c++) {
                votes[5*i+c]=0;
            }
        }
        for(unsigned i=0;i<=count;i++) {
            color_members m=membership.read();bool terminal=i==count;
            if(m.keep!=255 || m.strb!=255 || m.user!=(i==0) ||
                m.last!=terminal || m.id || m.dest)faults|=2;
            if(terminal) {if(m.data)faults|=2;}
            else {
                if(!m.data)faults|=2;
                for(unsigned k=0;k<64;k++) {
#pragma HLS PIPELINE II=1
                    if(m.data[k]) {
                        if(k>=n || owner[k])faults|=2;
                        owner[k]=i+1;
                    }
                }
            }
        }
        rows: for(unsigned y=0;y<h;y++) {
            scan: for(unsigned x=0;x<w;x++) {
#pragma HLS PIPELINE II=2
                color_pixel p=pixels.read();unsigned id=p.data.range(30,24);
                if(p.keep!=15 || p.strb!=15 || p.user!=(x==0 && y==0) ||
                    p.last!=(x+1==w) || p.id || p.dest || p.data[31] || id>n)faults|=4;
                ap_uint<4> slot=(id && id<=n)?owner[id-1]:ap_uint<4>(0);
                ap_uint<3> index=slot-1;
                ap_uint<3> vote=pixel_vote(p.data.range(23,0));
                if(slot) {
                    sampled[index]=sampled[index]+1;
                    // Exactly one color wins per pixel. A shared dual-port
                    // histogram avoids five replicated read/modify/write banks.
                    ap_uint<6> bin=ap_uint<6>(index)*5+ap_uint<3>(vote-1);
                    if(vote)votes[bin]=votes[bin]+1;
                }
            }
        }
        if(!faults)for(unsigned i=0;i<count;i++)if(!sampled[i])faults|=8;
        if(!faults)for(unsigned i=0;i<count;i++) {
            ap_uint<18> total=0,largest=0;ap_uint<3> best=0,ties=0;
            for(unsigned c=0;c<5;c++) {
                ap_uint<18> v=votes[5*i+c];total+=v;
                if(v>largest) {largest=v;best=c+1;ties=1;}
                else if(v==largest)ties++;
            }
            ap_uint<19> twice=ap_uint<19>(largest)<<1;
            ap_uint<3> label=total && ties==1 && twice>=total?best:ap_uint<3>(0);
            ap_uint<34> numerator=ap_uint<34>(largest)*ap_uint<16>(65535)+ap_uint<18>(total>>1);
            ap_uint<16> confidence=0;
            if(total)confidence=numerator/total;
            color_result r;r.data=0;r.data.range(2,0)=label;
            r.data.range(21,6)=confidence;r.data.range(39,22)=total;
            r.data.range(57,40)=sampled[i];r.keep=-1;r.strb=-1;
            r.user=i==0;r.last=0;r.id=0;r.dest=0;results.write(r);
        }
    }
    color_result t;t.data=0;t.keep=-1;t.strb=-1;t.user=faults || count==0;
    t.last=1;t.id=0;t.dest=0;results.write(t);status=faults;
}
