#pragma once


/**
用于动态加载dll/so的包装类
用法示例：

class User32DLL {
public:
    DYLIB_LOAD(User32DLL, "user32");
    DYLIB_IMPORT(MessageBoxA);
    DYLIB_IMPORT(MessageBoxW);
};


User32DLL user32;

user32.MessageBoxA(NULL, "Hello World", "test", MB_OK);

*/


#include <stdexcept>
#include <string>

#ifdef _WIN32

#include <windows.h>

static void _DylibLoad(const char* name, void** lib, int size) {
	HMODULE mod = LoadLibraryA(name);
	if (!mod) {
		std::string msg = "can not load library ";
		msg + name;
		throw std::runtime_error(msg);
	}
	for (int i = 0; i < size; i += 2) {
		const char* name = (const char*)(lib[i]);
		void* func = GetProcAddress(mod, name);
		if (!func) {
			std::string msg = "no function names ";
			msg + name;
			throw std::runtime_error(msg);
		}
		else {
			lib[i + 1] = func;
		}
	}
}


#else
#error "current not support"

#endif


#define _DYLIB_NAME_CAT(a,b) a##b


/** 声明一个导入类的构造函数 */
#define DYLIB_LOAD(cls, name) \
cls() {\
    _DylibLoad(name, (void**)this, sizeof(cls) / sizeof(void*));\
}

/** 声明一个导入函数 */
#define DYLIB_IMPORT(func) \
const char* _DYLIB_NAME_CAT(name_of_,func) = #func;\
decltype(::func)* func = NULL;






