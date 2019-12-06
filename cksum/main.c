#include "cksum.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>


int main(int argc, char* argv[]){    
    uint8_t buf[4096];
    FILE* f ;
    int len;
    uint32_t crc = 0;
    uint32_t total = 0; 
    if(argc<1){
        printf("usage cksum <file>\n");
        return 1;
    }
    f = fopen(argv[1],"rb");
    if(!f){
        perror("open file error");
        return 1;
    }
    while(1){
        len = fread(buf,1,4096,f);
        if(len<0 || len>4096){
            perror("read file error");
            return 1;            
        }
        if(len == 0)break;
        total += len;
        crc = cksum_update(crc, buf, len);
    }
    crc = cksum_final(crc, total);
    printf("%u %d %s", crc, total, argv[1]);
    return 0;
}

