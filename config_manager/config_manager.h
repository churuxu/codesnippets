#pragma once

#include <stdlib.h>
#include <stdint.h>


/*
配置参数管理

按key获取value

key支持多级,按/分隔



目前支持：
json文件

可扩展支持：
xml文件
ini文件
Windows注册表
...

*/


#ifdef __cplusplus
extern "C" {
#endif

typedef struct config config;

//加载配置
config* config_load(const char* data);

//释放配置
void config_free(config* cfg);

//获取字符串值
const char* config_get_string(config* cfg, const char* key);

//获取整数值
int config_get_integer(config* cfg, const char* key);

//获取子节点key
const char* config_enum_child(config* cfg, const char* parent, unsigned int index);

#ifdef __cplusplus
}
#endif


