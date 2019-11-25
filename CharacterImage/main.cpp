#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "ConsoleGui.h"
#include "CharacterImage.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"



static void* load_file_content(const char* name, size_t* plen) {
	void* ret = NULL;
	FILE* f = fopen(name, "rb");
	if (!f)return NULL;
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);	
	if (sz) {
		fseek(f, 0, SEEK_SET);
		void* data = malloc(sz);
		if (data) {
			int len = (int)fread(data, 1, sz, f);
			if (len > 0) {
				ret = data;
				if (plen)*plen = len;
			}
			else {
				free(data);
			}
		}
	}
	fclose(f);	
	return ret;
}


void help(){
	printf("usage: CharacterImage <ImageFile> [Width] [Height]\n");
}

int main(int argc, char* argv[]){

	if(argc<2){
		help();
		return 1;
	}

	//读取文件
	size_t len = 0;
	void* data = load_file_content(argv[1], &len);
	if(!data || !len){
		printf("can not load file:%s\n", argv[1]);
		return 2;
	}

	//加载图片
	void* pixel;
	int w, h, z, comp;
	int* delays;
	if(memcmp(data, "GIF", 3) == 0){ 
		pixel = stbi_load_gif_from_memory((stbi_uc*)data, (int)len, &delays, &w, &h, &z, &comp, 4);
	}else{
		z = 1;
		pixel = stbi_load_from_memory((stbi_uc*)data, (int)len, &w, &h, &comp, 0);
	}
	if(!pixel){
		printf("not image file\n");
		return 3;
	}	

	//各大小宽高计算
	int stride = w * comp;
	int pixellen = w * h * comp;
	int neww,newh;
	if(argc>2){ //指定了宽度
		neww = atoi(argv[2]);
		if(!neww)neww = 120;
	}else{ //未指定宽度
		neww = ConsoleGetWidth();
		if(!neww)neww = 80;
		if(w < neww) neww = w;	
	}
	if(argc>3){  //指定了高度
		newh = atoi(argv[3]);	
		if(!newh)newh = (h * neww / w) / 2;
	}else{ //未指定高度
		newh = (h * neww / w) / 2;
	}
	int newstride = neww * comp;
	int newpixellen = neww * newh * comp;
	int charlen = neww * newh + newh + 1;
	if(neww == ConsoleGetWidth()){ //控制台显示时直接铺满宽度，不要换行符
		CharacterImageSetLineBreak("");
	}

	//内存分配
	uint8_t* newpixel = (uint8_t*)malloc(newpixellen);
	if (!newpixel) {
		printf("out of memory\n");
		return 4;
	}	
	char* charimg = (char*)malloc(charlen);
	if (!charimg) {
		printf("out of memory\n");
		return 5;
	}

	//循环显示图片
	int ret;
	uint8_t* curpixel;
	int writelen = 0;
	int lastwrited = 0;
	int i = 0;
	while(1){
		curpixel = (uint8_t*)pixel + (i * pixellen);
		//图片缩放
		ret = stbir_resize_uint8(curpixel, w, h, stride, newpixel, neww, newh, newstride, comp);
		if (!ret) {
			printf("image resize error, [%d,%d] to [%d,%d]\n", w,h, neww, newh);
			return 6;
		}

		//转字符画
		writelen = CharacterImageCreate(newpixel, neww, newh, comp, charimg, charlen);
		if (writelen <= 0) {
			printf("create character image error [%d,%d]\n", neww, newh);
			return 7;
		}
		
		//显示
		if(lastwrited){
			ConsoleBackLine(newh);
		}
		//fwrite(charimg, 1, writelen, stdout);
		printf("%s", charimg);
		lastwrited = writelen;

		//非多帧gif，则退出		
		if(z == 1)break;

		//准备下一帧
		Sleep(delays[i] * 10);
		i ++;
		if(i == z)i = 0;		
	}

	return 0;
}




