#if defined(UNICODE) && !defined(_UNICODE) 
#define _UNICODE
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)  //win32
#include <tchar.h>
#include <windows.h>

#if defined(UNICODE) //unicode 

typedef wchar_t tchar;
#define tgetenv _wgetenv
#else   //ansi
typedef char tchar;
char* getenvutf8(char* name){	
	static char envbuf[1024];
    wchar_t namebuf[512];
    wchar_t valuebuf[512];
	int len,ret;
	envbuf[0]=0;
    len=MultiByteToWideChar(CP_UTF8,0,name,-1,namebuf,512);
    if(len<=0)return NULL;
    ret=GetEnvironmentVariableW(namebuf,valuebuf,512);
    if(!ret)return NULL;    
    len=WideCharToMultiByte(CP_UTF8,0,valuebuf,-1,envbuf,1024,NULL,NULL);
    if(len<=0)return NULL;
    return envbuf;
}
#define tgetenv getenvutf8
#endif //ansi

#else //linux
#define tgetenv getenv
#endif


void help(){
	printf("usage: envfr <input> <output>\n");
	printf("  replace file contents to environment value such as ${ENV_VAR}. \n");

}


#define abort_if(exp, msg) if(exp){printf("%s error:%s\n", msg, strerror(errno)); return 1;}


int envfr(const char* input, const char* output){
	FILE* in;
	FILE* out;
	long filesize;
	char* filedata; 
	int ret, len;
	tchar* next;
	tchar* cur;	
	int inenv = 0;
	tchar* defaultval;

	in = fopen(input, "rb");
	abort_if(!in,"open input file");
	out = fopen(output, "wb");
	abort_if(!out, "open output file");

	fseek(in, 0, SEEK_END);
	filesize = ftell(in);
	fseek(in, 0, SEEK_SET);

	filedata = (char*)malloc(filesize+2);
	abort_if(!filedata, "malloc");

	ret = (int)fread(filedata,1,filesize,in);
	abort_if(ret != filesize, "read file");

	filedata[ret] = 0;
	filedata[ret + 1] = 0;
	cur = (tchar*)filedata;
	next = cur;
	while(next){
		if(!inenv){
			next = _tcsstr(cur, _T("${"));
			if(next){
				*next = 0;
				next += 2;
				inenv = 1;					
			}
		}else{
			next = _tcschr(cur, '}');
			if(next){
				*next = 0;
				next += 1;
				defaultval = _tcsstr(cur, _T(":="));
				if(defaultval){
					*defaultval = 0;
					defaultval += 2;
				}
				cur = tgetenv(cur);
				if(!cur){
					cur = defaultval;
				}
				inenv = 0;	
			}
		}
		if(cur){
			len = _tcslen(cur) * sizeof(tchar);
			if(len){
				ret = fwrite(cur, 1, len, out);
				abort_if(ret != len, "write file");
			}
		}
		cur = next;
	}

	fclose(in);
	fclose(out);

	return 0;
}


int main(int argc, char* argv[]){
	if(argc<3){
		help();
		return 1;
	}
	return envfr(argv[1], argv[2]);	 
}


