#include "hexcodec.h"
#include <stdlib.h>

static const char hex_enc_map_ucase[] =
"0123456789ABCDEF";

static const char hex_enc_map_lcase[] =
"0123456789abcdef"; 

//-1=无效字符  -2=空白字符
static const int8_t hex_dec_map[]={
  // 0   1   2   3   4   5   6   7     8   9   A   B   C   D   E   F
    -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,  -1 ,-2 ,-2 ,-2 ,-1 ,-2 ,-1 ,-1 , //0
    -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,  -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 , //1
    -2 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,  -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 , //2
    0  ,1  ,2  ,3  ,4  ,5  ,6  ,7  ,  8  ,9  ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 , //3
    -1 ,10 ,11 ,12 ,13 ,14 ,15 ,-1 ,  -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 , //4
    -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,  -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 , //5
    -1 ,10 ,11 ,12 ,13 ,14 ,15 ,-1 ,  -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 , //6
    -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,  -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1   //7
};


int hex_encode_with_flag(const void* data, int datalen, char* buf, int buflen, int flag){
    const char* enc_map = (flag & HEX_ENCODE_FLAG_LOWERCASE)? hex_enc_map_lcase : hex_enc_map_ucase;
    int spacing = (flag & HEX_ENCODE_FLAG_SPACING)? 1 : 0;
    int i;
    uint8_t* ptr = (uint8_t*)data;
    char* out = buf;
    char* outend = buf + buflen;
    uint8_t b;
    if(!buf || buflen<=0)return 0;
    for(i=0; i<datalen; i++){
        b = *ptr;
        ptr ++;

        if(spacing && i){ //写入空格
            if(out >= outend)return 0; //buffer too small
            *out = ' ';
            out ++;
        }
        if(out >= outend)return 0; //buffer too small
        *out = enc_map[b >> 4];
        out ++;
        if(out >= outend)return 0; //buffer too small
        *out = enc_map[b & 0xf];
        out ++;        
    }
    *out = 0; 
    return out - buf;
}

int hex_encode(const void* data, int datalen, char* buf, int buflen){
    return hex_encode_with_flag(data, datalen, buf, buflen, 0);
}

//char 类型的作为数字下标识危险的
int hex_decode(const uint8_t* str, int strlen, void* buf, int buflen){
    uint8_t* ptr = (uint8_t*)str;
    uint8_t* ptrend = (uint8_t*)str + strlen;
    uint8_t* out = (uint8_t*)buf;
    uint8_t* outend = out + buflen;
    uint8_t c1,c2; 
    int8_t v1,v2;
    if(!buf || buflen<=0)return 0;
    while(1){
        c1 = *ptr;
        ptr ++;
        if(ptr > ptrend)break;        
        if(c1 > 127)return 0; //too big
        v1 = hex_dec_map[c1];
        if(v1 == -1)return 0; //invalid
        if(v1 == -2)continue; //blank char        
        c2 = *ptr;
        ptr ++;        
        if(c2 > 127)return 0; //too big
        v2 = hex_dec_map[c2];
        if(v2 == -1)return 0; //invalid
        if(v2 == -2)return 0; //blank char is invalid

        if(out >= outend)return 0; //buffer too small
        *out = (v1<<4)|v2;
        out ++;
        if(ptr >= ptrend)break;
    }
    return out - (uint8_t*)buf;
}



