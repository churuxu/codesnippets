#include <windows.h>
#include "DylibWrap.hpp"


class User32DLL {
public:
	DYLIB_LOAD(User32DLL, "user32");
	DYLIB_IMPORT(MessageBoxA);
	DYLIB_IMPORT(MessageBoxW);
};


User32DLL user32;

int main() {
	

	user32.MessageBoxA(NULL, "Hello World 111", "test1", MB_OK);
	user32.MessageBoxW(NULL, L"Hello World 222", L"test2", MB_OK);
	return 0;
}
