#include "lumen/canvas.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <stdexcept>
namespace lumen {
Canvas::Canvas(Size size, Color clear_color) : size_(size) {
    if (size.width < 0 || size.height < 0) throw std::invalid_argument("Canvas dimensions cannot be negative");
    pixels_.resize(static_cast<std::size_t>(size.width) * static_cast<std::size_t>(size.height) * 4);
    clear(clear_color);
}
void Canvas::clear(Color c) {
    for (std::size_t i=0;i<pixels_.size();i+=4) { pixels_[i]=c.r; pixels_[i+1]=c.g; pixels_[i+2]=c.b; pixels_[i+3]=c.a; }
}
Color Canvas::pixel(Point p) const noexcept {
    if (p.x<0 || p.y<0 || p.x>=size_.width || p.y>=size_.height) return transparent;
    const auto i=(static_cast<std::size_t>(p.y)*size_.width+p.x)*4;
    return {pixels_[i],pixels_[i+1],pixels_[i+2],pixels_[i+3]};
}
void Canvas::put_pixel(int x,int y,Color c) {
    if (x<0 || y<0 || x>=size_.width || y>=size_.height || c.a==0) return;
    const auto i=(static_cast<std::size_t>(y)*size_.width+x)*4;
    if (c.a==255) { pixels_[i]=c.r; pixels_[i+1]=c.g; pixels_[i+2]=c.b; pixels_[i+3]=255; return; }
    const unsigned a=c.a, inv=255-a, da=pixels_[i+3];
    const unsigned out_a=a+(da*inv+127)/255;
    if (!out_a) return;
    pixels_[i]=static_cast<std::uint8_t>((c.r*a+pixels_[i]*da*inv/255)/out_a);
    pixels_[i+1]=static_cast<std::uint8_t>((c.g*a+pixels_[i+1]*da*inv/255)/out_a);
    pixels_[i+2]=static_cast<std::uint8_t>((c.b*a+pixels_[i+2]*da*inv/255)/out_a);
    pixels_[i+3]=static_cast<std::uint8_t>(out_a);
}
void Canvas::fill_rect(Rect r,Color c) {
    const int x0=std::max(0,r.x), y0=std::max(0,r.y);
    const int x1=std::min(size_.width,r.x+std::max(0,r.width)), y1=std::min(size_.height,r.y+std::max(0,r.height));
    for(int y=y0;y<y1;++y) for(int x=x0;x<x1;++x) put_pixel(x,y,c);
}
void Canvas::stroke_rect(Rect r,Color c,int t) {
    if(r.empty() || t<=0) return;
    t=std::min(t,std::min(r.width,r.height));
    fill_rect({r.x,r.y,r.width,t},c); fill_rect({r.x,r.y+r.height-t,r.width,t},c);
    fill_rect({r.x,r.y,t,r.height},c); fill_rect({r.x+r.width-t,r.y,t,r.height},c);
}
namespace {
using Glyph=std::array<std::uint8_t,7>;
Glyph glyph(char c) {
    switch(static_cast<char>(std::toupper(static_cast<unsigned char>(c)))) {
#define G(ch,a,b,c,d,e,f,g) case ch: return {a,b,c,d,e,f,g}
    G('A',14,17,17,31,17,17,17); G('B',30,17,17,30,17,17,30); G('C',14,17,16,16,16,17,14);
    G('D',30,17,17,17,17,17,30); G('E',31,16,16,30,16,16,31); G('F',31,16,16,30,16,16,16);
    G('G',14,17,16,23,17,17,15); G('H',17,17,17,31,17,17,17); G('I',14,4,4,4,4,4,14);
    G('J',7,2,2,2,18,18,12); G('K',17,18,20,24,20,18,17); G('L',16,16,16,16,16,16,31);
    G('M',17,27,21,21,17,17,17); G('N',17,25,21,19,17,17,17); G('O',14,17,17,17,17,17,14);
    G('P',30,17,17,30,16,16,16); G('Q',14,17,17,17,21,18,13); G('R',30,17,17,30,20,18,17);
    G('S',15,16,16,14,1,1,30); G('T',31,4,4,4,4,4,4); G('U',17,17,17,17,17,17,14);
    G('V',17,17,17,17,17,10,4); G('W',17,17,17,21,21,21,10); G('X',17,17,10,4,10,17,17);
    G('Y',17,17,10,4,4,4,4); G('Z',31,1,2,4,8,16,31);
    G('0',14,17,19,21,25,17,14); G('1',4,12,4,4,4,4,14); G('2',14,17,1,2,4,8,31);
    G('3',30,1,1,14,1,1,30); G('4',2,6,10,18,31,2,2); G('5',31,16,16,30,1,1,30);
    G('6',14,16,16,30,17,17,14); G('7',31,1,2,4,8,8,8); G('8',14,17,17,14,17,17,14);
    G('9',14,17,17,15,1,1,14);
#undef G
    case '-': return {0,0,0,31,0,0,0}; case '_': return {0,0,0,0,0,0,31};
    case '.': return {0,0,0,0,0,12,12}; case ':': return {0,12,12,0,12,12,0};
    case '!': return {4,4,4,4,4,0,4}; case '?': return {14,17,1,2,4,0,4};
    case '/': return {1,2,2,4,8,8,16}; case '+': return {0,4,4,31,4,4,0};
    case ' ': return {}; default: return {31,17,5,9,4,0,4};
    }
}
}
void Canvas::draw_text(Point p,std::string_view text,Color c,int scale) {
    if(scale<1) return;
    int x=p.x;
    for(char ch:text) {
        const auto rows=glyph(ch);
        for(int y=0;y<7;++y) for(int col=0;col<5;++col)
            if(rows[static_cast<std::size_t>(y)] & (1u << (4-col))) fill_rect({x+col*scale,p.y+y*scale,scale,scale},c);
        x+=6*scale;
    }
}
}
