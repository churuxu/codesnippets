#include "generic_packet.h"
#include <string.h>
#include <stdio.h>

#define MAX_NAME  32
#define MAX_VALUE 32


static const uint8_t BCD_TO_HEX[256] = {
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    10,11,12,13,14,15,16,17,18,19, 0, 0, 0, 0, 0, 0,
    20,21,22,23,24,25,26,27,28,29, 0, 0, 0, 0, 0, 0,
    30,31,32,33,34,35,36,37,38,39, 0, 0, 0, 0, 0, 0,
    40,41,42,43,44,45,46,47,48,49, 0, 0, 0, 0, 0, 0,
    50,51,52,53,54,55,56,57,58,59, 0, 0, 0, 0, 0, 0,
    60,61,62,63,64,65,66,67,68,69, 0, 0, 0, 0, 0, 0,
    70,71,72,73,74,75,76,77,78,79, 0, 0, 0, 0, 0, 0,
    80,81,82,83,84,85,86,87,88,89, 0, 0, 0, 0, 0, 0,
    90,91,92,93,94,95,96,97,98,99, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


//获取下一段字符串
//"u8 u16 u16" 获取到 u8, 返回剩余字符串
static const char* next_token(const char* str, char* value/*32*/, char* name/*32*/){       
    const char* ptr = str;
    char ch,firstch;
    int started;
    int vallen, namelen;
    int invalue;
    if(!*ptr)return NULL;   
    started = 0;
    firstch = 0;
    invalue = 0;
    vallen = 0;
    namelen = 0;
    while(*ptr){
        ch = *ptr;
        if(ch == ' '){
            if(started){
                break;
            }
        }else{
            if(!firstch){
                firstch = ch;
                started = 1;
                if(ch == '$'){
                    invalue = 0;
                    ptr ++; continue;
                }else{
                    invalue = 1;
                }                
            }
            if(ch == '|'){
                invalue = 1;
                ptr ++; continue;
            }
            if(invalue){
                if(vallen >= MAX_VALUE)return NULL;
                value[vallen] = ch;
                vallen ++;
            }else{
                if(namelen >= MAX_NAME)return NULL;
                name[namelen] = ch;
                namelen ++;
            }
        }
        ptr ++;
    }
    if(vallen >= MAX_VALUE)return NULL;
    if(namelen >= MAX_NAME)return NULL;
    value[vallen] = 0;
    name[namelen] = 0;
    return ptr;
}

//转换大小端函数
static uint16_t reverse_u16(uint16_t value){  
    return (value & 0x00FF) << 8 | (value & 0xFF00U) >> 8 ;  
}
static uint32_t reverse_u32(uint32_t value){  
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |   
        (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;   
}
static uint64_t reverse_u64(uint64_t value){  
    uint64_t high_uint64 = (uint64_t)reverse_u32((uint32_t)(value));
    uint64_t low_uint64 = (uint64_t)reverse_u32((uint32_t)(value >> 32)); 
    return (high_uint64 << 32) + low_uint64;  
} 


//u\i开头，整数
static int convert_int_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    int ret = -1;
    packet_integer_t v = 0;
    switch (fmt[0]){
    case 'u':
        switch (fmt[1]){
        case '8': v = *(uint8_t*)buf; ret = 1; break;
        case '1': v = *(uint16_t*)buf; ret = 2; break;
        case '3': v = *(uint32_t*)buf; ret = 4; break;
        case '6': v = *(uint64_t*)buf; ret = 8; break; 
        default:break;
        }
        break;
    case 'U':
        switch (fmt[1]){
        case '8': v = *(uint8_t*)buf; ret = 1; break;
        case '1': v = *(uint16_t*)buf; v = reverse_u16(v); ret = 2; break;
        case '3': v = *(uint32_t*)buf; v = reverse_u32(v); ret = 4; break;
        case '6': v = *(uint64_t*)buf; v = reverse_u64(v); ret = 8; break; 
        default:break;
        }
        break;
    case 'i':
        switch (fmt[1]){
        case '8': v = *(int8_t*)buf; ret = 1; break;
        case '1': v = *(int16_t*)buf; ret = 2; break;
        case '3': v = *(int32_t*)buf; ret = 4; break;
        case '6': v = *(int64_t*)buf; ret = 8; break; 
        default:break;
        }
        break;
    case 'I':
        switch (fmt[1]){
        case '8': v = *(int8_t*)buf; ret = 1; break;
        case '1': v = *(int16_t*)buf; v = reverse_u16(v); ret = 2; break;
        case '3': v = *(int32_t*)buf; v = reverse_u32(v); ret = 4; break;
        case '6': v = *(int64_t*)buf; v = reverse_u64(v); ret = 8; break; 
        default:break;
        }
        break;        
    default:break;
    }
    if(ret>0){
        val->type = PACKET_VALUE_INTEGER;
        val->len = 0;
        val->int_val = v;
    }
    return ret;
}
//f开头，浮点数
static int convert_f_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){    
    int ret = -1;
    packet_number_t v = 0;
    switch (fmt[1]){
        case '3': v = (packet_number_t)*(float*)buf; ret = 4; break;
        case '6': v = (packet_number_t)*(double*)buf; ret = 8; break;
        default:break;
    }    
    if(ret>0){
        val->type = PACKET_VALUE_NUMBER;
        val->len = 0;
        val->num_val = v;
    }
    return ret;
}
//B开头，二进制
static int convert_b_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    int len = atoi(fmt + 1);
    val->bin_val = buf;
    val->len = len;
    val->type = PACKET_VALUE_BINARY;
    return len;
}
//bcd码
static int convert_bcd_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    int len = atoi(fmt + 1);
    int i;
    uint8_t data;
    packet_integer_t v = 0;
    for(i=0;i<len;i++){
        data = buf[i];        
        v = v*100 + BCD_TO_HEX[data];
    }
    val->type = PACKET_VALUE_INTEGER;
    val->len = 0;
    val->int_val = v;
    return len;
}

//D H开头，字符串形式数字
static int convert_n_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    packet_integer_t v = 0;
    int radix = (fmt[0] == 'H')?16:10; 
    char temp[32]; 
    char* end = NULL;
    char* start = temp;
    int len = -1;
    int minlen;
    int ret = -1; 
    if(fmt[1] == '*'){ //非固定长度字符串
        minlen = (buflen >= 31)?31:buflen;
    }else{ //固定长度
        len = atoi(fmt + 1);
        if(len>31)return -1;
        minlen = (buflen >= len)?len:buflen;
    }
    memcpy(temp, buf, minlen);
    temp[minlen] = 0;
    v = strtol(temp, &end, radix);
    if(fmt[1] == '*'){ //非固定长度
        ret = end - start;
        if(!ret)ret = -1;
    }else{ //固定长度
        ret = len;
    }
    val->type = PACKET_VALUE_INTEGER;
    val->len = 0;
    val->int_val = v;
    return ret;
}

//将数据转换成实际所需的值, 返回用掉的buf长度，失败返回<=0
static int convert_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    switch(fmt[0]){
        case 'u':
        case 'U':            
        case 'i':
        case 'I':
            return convert_int_data(fmt, buf, buflen, val);
        case 'f':
            return convert_f_data(fmt, buf, buflen, val);
        case 'D':
        case 'H':
            return convert_n_data(fmt, buf, buflen, val); 
        case 'B':
            return convert_b_data(fmt, buf, buflen, val);
        case 'C':
            return convert_bcd_data(fmt, buf, buflen, val);            
        default:
            break;               
    }
    return -1;
}

//解析
int packet_parse(const char* format, void* buf, int buflen, packet_callback cb, void* udata){
    char name[MAX_NAME];
    char fmt[MAX_VALUE];
    int ret, remainlen, err;
    uint8_t* pbuf = (uint8_t*) buf;
    packet_value val;
    const char* ptr = format;  
    
    remainlen = buflen;
    while(remainlen > 0){
        ptr = next_token(ptr, fmt, name);
        if(!ptr)break;
        ret = convert_data(fmt, pbuf, remainlen, &val);
        if(ret<=0)return ret;
        if(name[0]){ //需要回调
            err = cb(udata, name, &val);
            if(err)return err;
        }
        remainlen -= ret;
        pbuf += ret;
    }
    return 0;
}



//写入整数数据
static int write_int_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    packet_integer_t v;
    if(val->type == PACKET_VALUE_INTEGER){
        v = val->int_val;
    }else if(val->type == PACKET_VALUE_NUMBER){
        v = (packet_integer_t)val->num_val;
    }else{
        return -1;
    }
    switch (fmt[0]){
    case 'u':
        switch (fmt[1]){
        case '8': if(buflen<1)return -1; *(uint8_t*)buf = (uint8_t)v; return 1;
        case '1': if(buflen<2)return -1; *(uint16_t*)buf = (uint16_t)v; return 2;
        case '3': if(buflen<4)return -1; *(uint32_t*)buf = (uint32_t)v; return 4;
        case '6': if(buflen<8)return -1; *(uint64_t*)buf = (uint64_t)v; return 8;
        default:break;
        }
        break;
    case 'U':
        switch (fmt[1]){
        case '8': if(buflen<1)return -1; *(int8_t*)buf = (int8_t)v; return 1;
        case '1': if(buflen<2)return -1; *(int16_t*)buf = reverse_u16((int16_t)v); return 2;
        case '3': if(buflen<4)return -1; *(int32_t*)buf = reverse_u32((int32_t)v); return 4;
        case '6': if(buflen<8)return -1; *(int64_t*)buf = reverse_u64((int64_t)v); return 8; 
        default:break;
        }
        break;
    case 'i':
        switch (fmt[1]){
        case '8': if(buflen<1)return -1; *(int8_t*)buf = (int8_t)v; return 1;
        case '1': if(buflen<2)return -1; *(int16_t*)buf = (int16_t)v; return 2;
        case '3': if(buflen<4)return -1; *(int32_t*)buf = (int32_t)v; return 4;
        case '6': if(buflen<8)return -1; *(int64_t*)buf = (int64_t)v; return 8; 
        default:break;
        }
        break;
    case 'I':
        switch (fmt[1]){
        case '8': if(buflen<1)return -1; *(int8_t*)buf = (int8_t)v; return 1;
        case '1': if(buflen<2)return -1; *(int16_t*)buf = reverse_u16((int16_t)v); return 2;
        case '3': if(buflen<4)return -1; *(int32_t*)buf = reverse_u32((int32_t)v); return 4;
        case '6': if(buflen<8)return -1; *(int64_t*)buf = reverse_u64((int64_t)v); return 8; 
        default:break;
        }
        break;        
    default:break;
    }
    return -1;
}

//写入实数数据
static int write_f_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    packet_number_t v;
    if(val->type == PACKET_VALUE_INTEGER){
        v = (packet_number_t)val->int_val;
    }else if(val->type == PACKET_VALUE_NUMBER){
        v = val->num_val;
    }else{
        return -1;
    }
    switch (fmt[1]){
        case '3': if(buflen<4)return -1; *(float*)buf = (float)v; return 4;
        case '6': if(buflen<8)return -1; *(double*)buf = (double)v; return 8;
        default:break;
    }
    return -1;       
}

//写入字符串形式整数
static int write_n_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    packet_integer_t v = 0;
    const char* pfmt = (fmt[0] == 'H')?"%X":"%d"; 
    char temp[32]; 
    int len = -1;

    if(val->type == PACKET_VALUE_INTEGER){
        v = val->int_val;
    }else if(val->type == PACKET_VALUE_NUMBER){
        v = (packet_integer_t)val->num_val;
    }else{
        return -1;
    }

    if(fmt[1] == '*'){ //非固定长度字符串
        len = snprintf((char*)buf, buflen, pfmt, v);
        /*
        if(fmt[0] == 'H'){
            return snprintf((char*)buf, buflen, pfmt, v);
        }else{
            return snprintf((char*)buf, buflen, "%d", v);
        }   */     
    }else{ //固定长度
        len = atoi(fmt + 1);
        if(len>31)return -1;
        snprintf(temp,32,"%%0%d%s", len, ((fmt[0] == 'H')?"X":"d"));
        len = snprintf((char*)buf, buflen, temp, v);
    }
    return len-1;       
}


//将值写入到buf, 返回写入的长度
static int write_data(const char* fmt, uint8_t* buf, int buflen, packet_value* val){
    switch(fmt[0]){
        case 'u':
        case 'U':            
        case 'i':
        case 'I':
            return write_int_data(fmt, buf, buflen, val);
        case 'f':
            return write_f_data(fmt, buf, buflen, val);  
        case 'D':
        case 'H':
            return write_n_data(fmt, buf, buflen, val);                  
        default:
            break;               
    }
    return -1;    
}



//组装报文
int packet_build(const char* format, void* buf, int buflen, packet_callback cb, void* udata){
    char name[MAX_NAME];
    char fmt[MAX_VALUE];
    int ret, remainlen, err;
    uint8_t* pbuf = (uint8_t*) buf;
    packet_value val;
    const char* ptr = format;  
    uint8_t b;
    remainlen = buflen;
    ret = 0;    
    while(remainlen > 0){
        ptr = next_token(ptr, fmt, name);
        if(!ptr)break;
        if(name[0]){ //需要从变量获取
            //从变量获取            
            val.len = ret;
            val.bin_val = buf;
            err = cb(udata, name, &val);
            if(err)return -1;
            //转换成指定格式
            ret = write_data(fmt, pbuf, remainlen, &val);
            if(ret<=0)return -1;
        }else{ //直接是报文值            
            b = strtol(fmt, NULL, 16);
            *pbuf = b;
            ret = 1;
        }
        pbuf += ret;        
        remainlen -= ret;
    }
    return pbuf - (uint8_t*)buf;
}








