#include "config_manager.h"
#include "json.h"
#include <string.h>
#include <stdio.h>


struct config{
    json_value* json;
    char num[16];
};

//加载文件
static char* load_file(const char* file, size_t* psz) {
	char* buf = NULL;	
    long sz;
	FILE* f = fopen(file, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		sz = ftell(f);
		fseek(f, 0, SEEK_SET);
		if (sz) {	
			buf = (char*)malloc(sz);
			if (buf) {
				int iret = fread(buf, 1, sz, f);
				if(iret != sz){
                    free(buf);
                    buf = NULL; 
                }else{
                    *psz = sz;
                }
			}
		}
		fclose(f);
	}
	return buf;
}

//加载配置
config* config_load(const char* name){
    config* ret ;
    char * data;
    size_t sz;
    json_value* val;
    if (!name)return NULL;
    if(*name == '{' || *name == '['){
        val = json_parse(name, strlen(name));
    }else{
        data = load_file(name, &sz);
        if(!data)return NULL;
        val = json_parse(data, sz);
        free(data);
    }    
    if(!val)return NULL;
    ret = (config* )malloc(sizeof(config));
    if(!ret){
        json_value_free(val);
        return NULL;
    }
    ret->json = val;
    ret->num[0] = 0;
    return ret;
}

//释放配置
void config_free(config* cfg){
    if(cfg)free(cfg);
}


//json value 按key查找值，单级
json_value* json_find_value_single(json_value* json, const char* key, unsigned int keylen){
    unsigned int i;    
    if(json->type == json_object){ 
        for (i = 0; i < json->u.object.length; i++ ){
            if(json->u.object.values[i].name_length == keylen && 0 == memcmp(json->u.object.values[i].name, key, keylen)){
                return json->u.object.values[i].value;
            }
        }
    }else if(json->type == json_array){
        i = strtoul(key, NULL, 10);
        if(i < json->u.array.length){
            return json->u.array.values[i];
        }
    }
    return NULL;    
}

//json value 按key查找值, 支持多级 ( a/b/c)
json_value* json_find_value(json_value* json,  const char* key){    
    const char* pkey;
    const char* pnext;
    json_value* cur;
    int last = 0;
    int keylen;
    if(!json || !key)return NULL;
    pkey = key;    
    cur = json;
    while(1){
        //找是否还有 /
        pnext = strchr(pkey, '/');
        if(pnext){
            keylen = pnext - pkey;                                
        }else{
            last = 1;
            keylen = strlen(pkey);
        }
        if(keylen==0)return NULL; //key错误  

        //按第一段作为key，查找值 
        cur = json_find_value_single(cur, pkey, keylen);
        if(!cur)return NULL;
                
        if(last){ //如果是最后一段
            return cur;
        }else{ //不是最后一段
            pkey = pnext + 1;
        }
    }
    return NULL;
}



//获取参数，字符串
const char* config_get_string(config* cfg, const char* key){
    json_value* val;
    if(!cfg || !key)return NULL;
    val = json_find_value(cfg->json, key);
    if(!val)return NULL;
    if(val->type == json_string){
        return val->u.string.ptr;
    }
    return NULL;
}

//获取参数，整数
int config_get_integer(config* cfg, const char* key){
    json_value* val;
    if(!cfg || !key)return 0;
    val = json_find_value(cfg->json, key);
    if(!val)return 0;
    if(val->type == json_integer){
        return (int)val->u.integer;
    }
    return 0;
}

//获取子key列表
const char* config_enum_child(config* cfg, const char* parent, unsigned int index){
    json_value* json;
    if(!cfg)return NULL;
    if(!parent || !(*parent)){
        json = cfg->json;
    }else{
        json = json_find_value(cfg->json, parent);
    }    
    if(!json)return NULL;
    if(json->type == json_object){  //object 直接返回 key
        if(index < json->u.object.length){
            return json->u.object.values[index].name;
        }
        return NULL;
    }else if(json->type == json_array){ //array 将索引转字符串再返回
        if(index < json->u.array.length){
           snprintf(cfg->num, 16, "%u", index);
           return cfg->num;
        }
        return NULL;
    }
    return NULL;
}



