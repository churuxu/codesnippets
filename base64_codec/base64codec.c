#include "base64codec.h"


static const char base64_enc_map[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char base64_dec_map[]={
  // 0   1   2   3   4   5   6   7     8   9   A   B   C   D   E   F
    -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,  -1 ,127,127,127,127,127,-1 ,-1 , //0
    -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,  -1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 , //1
    127,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,  -1 ,-1 ,-1 ,62 ,-1 ,-1 ,-1 ,63 , //2
    52 ,53 ,54 ,55 ,56 ,57 ,58 ,59 ,  60 ,61 ,-1 ,-1 ,-1 ,64 ,-1 ,-1 , //3
    -1 ,0  ,1  ,2  ,3  ,4  ,5  ,6  ,  7  ,8  ,9  ,10 ,11 ,12 ,13 ,14 , //4
    15 ,16 ,17 ,18 ,19 ,20 ,21 ,22 ,  23 ,24 ,25 ,-1 ,-1 ,-1 ,-1 ,-1 , //5
    -1 ,26 ,27 ,28 ,29 ,30 ,31 ,32 ,  33 ,34 ,35 ,36 ,37 ,38 ,39 ,40 , //6
    41, 42 ,43 ,44 ,45 ,46 ,47 ,48 ,  49 ,50 ,51 ,-1 ,-1 ,-1 ,-1 ,-1 , //7
};



//编码，返回编码后字符串长度
int base64_encode(const void* data, int datalen, char* buf, int buflen){
    uint8_t* pin = (uint8_t*)data;    
    uint8_t* pin_end = pin + datalen;
    char* pout = buf;
    char* pout_end = pout + buflen;
    uint8_t a,b,c;
   
    while(1){
        if(pin_end - pin < 3) break; //结束
        if(pout_end - pout < 4)return -1; //buf不够

        a = pin[0];
        b = pin[1];
        c = pin[2];        
        pout[0] = base64_enc_map[a>>2]; 
        pout[1] = base64_enc_map[((a&0x03)<<4)|(b>>4)]; 
        pout[2] = base64_enc_map[((b&0x0F)<<2)|(c>>6)];
        pout[3] = base64_enc_map[c&0x3F];

        pout += 4;
        pin += 3;       
    }

    if(pin_end != pin && pout_end - pout < 4)return -1; //buf不够

    switch (pin_end - pin){
    case 2:
        a = pin[0];
        b = pin[1];
		pout[0] = base64_enc_map[a>>2];	    
		pout[1] = base64_enc_map[((a&0x03)<<4)|(b>>4)];
		pout[2] = base64_enc_map[(b&0x0F)<<2];
		pout[3] = '=';
        pout += 4;
        break;
    case 1:
        a = pin[0];
	    pout[0] = base64_enc_map[a>>2];
	    pout[1] = base64_enc_map[(a&0x03)<<4];
		pout[2] = '=';
		pout[3] = '=';
        pout += 4; 
        break;
    default:
        break;
    }
    if(pout < pout_end)*pout = 0;
    return pout - buf;
}


//解码，返回解码后字节长度，失败返回0
int base64_decode(const char* str, int len, void* buf, int buflen){
    char* pin = (char*)str;    
    char* pin_end = pin + len;
    uint8_t* pout = (uint8_t*)buf;
    uint8_t* pout_end = pout + buflen;
    char ch;
    int cur = 0;
    int end = 0;
    char a[4];
    while(1){
        if(pin >= pin_end) break;

        ch = *pin;
        pin ++;
        if(ch < 0)return -1; //错误字符
        a[cur] = base64_dec_map[(uint8_t)ch];
        if(a[cur] == (char)127)continue; //空格
        if(a[cur] == (char)-1)return -1; //错误字符
        if(a[cur] == (char)64)end=1;     //'='结尾 

        if(end){
            if(a[cur]>=0 && a[cur]<64){
                return -1;  //中间包含'='
            }else{
                continue; 
            }
        }

        if(cur == 3){
            if(pout_end - pout < 3)return -1; //buf不够
            pout[0] = (a[0]<<2)|(a[1]>>4);
            pout[1] = (a[1]<<4)|(a[2]>>2);
            pout[2] = (a[2]<<6)|(a[3]);
            pout += 3;
            cur = 0;
        }else{
            cur++;
        }        
    }

	switch(cur){	    
	    case 1: // x===
	        return -1; 
	    case 2:  // xx==
            pout[0] = (a[0]<<2)|(a[1]>>4);
            pout += 1;
	        break;
	    case 3:  // xxx=
	        pout[0] =(a[0]<<2)|(a[1]>>4);
            pout[1] =(a[1]<<4)|(a[2]>>2); 
            pout += 2;
		    break;
		default:
		    break;
	}    

    
    return pout - (uint8_t*)buf;
}




