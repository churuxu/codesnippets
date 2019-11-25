#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//设置所用字符（从深到浅排列）
void CharacterImageSetCharacters(const char* str);

//设置换行符号（默认\n）
void CharacterImageSetLineBreak(const char* str);

//创建字符画，指定参数：像素，宽度，高度，每像素字节，输出buf，输出buf长度，返回结果长度，错误返回-1；
int CharacterImageCreate(void* pixel, int w, int h, int comp, char* buf, int buflen);


#ifdef __cplusplus
}
#endif

