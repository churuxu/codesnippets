#include "CharacterImage.h"
#include <string>
#include <stdlib.h>
#include <string.h>

static std::string basechars_ = "M@NH$W%#QU38hOpxes?oi[t=+)<!~^;:'. ";
static std::string linebreak_ = "\n";

void CharacterImageSetCharacters(const char* str){
    if(str)basechars_ = str;
}


void CharacterImageSetLineBreak(const char* str){
    if(str)linebreak_ = str;
}

//单个像素转字符
char PixeltoCharacter(uint8_t* pixel, int comp){
    if(comp == 4 && pixel[3]<0x10)return ' '; //透明
    int gray = ((int)(pixel[0])*30 + (int)(pixel[1])*59 + (int)(pixel[2])*11 + 50) / 100;
    int len = (int)basechars_.length();
    int index = gray * len / 256;
    if(index < 0 || index > len)return ' '; //错误
    return basechars_[index];
}


int CharacterImageCreate(void* pixel, int w, int h, int comp, char* buf, int buflen){
    uint8_t* ptr = (uint8_t*)pixel;
    char* out = buf;
    int x, y;    
    int needlen = w * h + ((int)linebreak_.length() * h);
    if(!buf && buflen == 0)return needlen;
    if(needlen > buflen)return -1;
	if(comp != 3 && comp != 4)return -1;
    for(y = 0; y < h; y++){
        for(x = 0; x < w; x++){
            *out = PixeltoCharacter(ptr, comp);
            out += 1;
            ptr += comp;
        }
        if(linebreak_.length()){
            memcpy(out, linebreak_.c_str(), linebreak_.length());
            out += linebreak_.length();
        }       
    }
    if(needlen < buflen){
        *out = 0;
    }
    return needlen;
}




