#include "color.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
static unsigned word(FILE* f){unsigned v;if(fread(&v,4,1,f)!=1)std::exit(20);return v;}
static unsigned long long wide(FILE* f){unsigned long long v;if(fread(&v,8,1,f)!=1)std::exit(21);return v;}
template<class T>static T packet(unsigned long long d,bool user,bool last) {
    T p;p.data=d;p.user=user;p.last=last;p.keep=-1;p.strb=-1;p.id=0;p.dest=0;return p;
}
int main(){
    std::ifstream list("cases.txt");std::string name;unsigned tests=0;
    while(list>>name) {
#ifdef COLOR_RTL_TEST
        if(name!="history_0" && name!="random_high64" && name!="tie_unknown" && name!="late_pixel_fault" && name!="membership_terminal_fault")continue;
#endif
        FILE* f=fopen((name+".bin").c_str(),"rb");if(!f)return 22;
        unsigned w=word(f),h=word(f),n=word(f),count=word(f),fault=word(f),mode=word(f);
        hls::stream<color_members> members;hls::stream<color_pixel> pixels;hls::stream<color_result> out;
        for(unsigned i=0;i<count;i++)members.write(packet<color_members>(wide(f),i==0,false));
        members.write(packet<color_members>(mode==2?1:0,count==0,true));
        for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++) {
            color_pixel p=packet<color_pixel>(word(f),x==0 && y==0,x+1==w);
            if(mode==1 && x+1==w && y+1==h)p.last=0;
            pixels.write(p);
        }
        ap_uint<4> status;live_object_color(members,pixels,out,w,h,n,count,status);
        if(status!=fault || !members.empty() || !pixels.empty()){printf("STATUS_FAIL %s got=%u expected=%u\n",name.c_str(),status.to_uint(),fault);return 23;}
        unsigned outputs=fault?0:count;
        for(unsigned i=0;i<=outputs;i++) {
            bool terminal=i==outputs;unsigned long long expected=terminal?0:wide(f);
            if(out.empty())return 24;color_result r=out.read();
            if(r.data!=expected || r.keep!=255 || r.strb!=255 || r.id || r.dest || r.last!=terminal || r.user!=(i==0)) {
                printf("RESULT_FAIL %s slot=%u got=%llu expected=%llu\n",name.c_str(),i,r.data.to_uint64(),expected);return 25;
            }
        }
        if(!out.empty() || fgetc(f)!=EOF)return 26;fclose(f);tests++;printf("COLOR_CASE_PASS %s\n",name.c_str());
    }
    // Invalid dimensions must emit a terminal without reading streams.
    hls::stream<color_members> m;hls::stream<color_pixel> p;hls::stream<color_result> o;ap_uint<4> s;
    live_object_color(m,p,o,0,1,0,0,s);if(s!=1 || o.empty())return 27;
    color_result t=o.read();if(t.data || !t.last || !t.user || !o.empty())return 28;
#ifdef COLOR_RTL_TEST
    if(tests!=5)return 29;puts("COLOR_RTL_PASS cases=6");
#else
    if(tests!=15)return 29;puts("COLOR_ALL_PASS cases=16");
#endif
    return 0;
}
