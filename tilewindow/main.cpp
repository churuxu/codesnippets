#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <string>

static int w_;
static int h_;
static RECT srcrc_;
static std::wstring key_;
static int x_;
static int y_;


void tile_next_window(HWND hwnd) {
	int x = x_;
	int y = y_;
	printf("MoveWindow %p %d %d %d %d\n", hwnd,x, y, w_, h_);
	MoveWindow(hwnd, x, y, w_, h_, TRUE);
	x_ += w_;
	if (x_ + w_ > srcrc_.right) {
		x_ = 0;
		y_ += h_;
	}
}

BOOL CALLBACK OnEnumWindowsProc(HWND hwnd, LPARAM lParam) {
	TCHAR title[1024];
	//int len = GetClassName(hwnd, title, 512);
	int len = GetWindowText(hwnd, title, 1024);
	if (len>0) {
		std::wstring thistitle = title;
		if (thistitle.find(key_) != std::string::npos) {
			tile_next_window(hwnd);
		}
	}
	return TRUE;
}

int tilewindow(LPCTSTR filter, int w, int h) {
	w_ = w;
	h_ = h;
	key_ = filter;
	BOOL bret = SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&srcrc_, 0);
	if (!bret) {
		printf("SystemParametersInfo error %d\n",GetLastError());
		return 1;
	}
	x_ = srcrc_.left;
	y_ = srcrc_.top;

	EnumWindows(OnEnumWindowsProc, NULL);

	return 0;
}

void showhelp() {
	printf("usage: tilewindow <title> <width> <height>\n");
}


int _tmain(int argc, TCHAR* argv[]) {
	int w, h;
	if (argc < 4) {
		showhelp();
		return 1;
	}
	w = _ttoi(argv[2]);
	h = _ttoi(argv[3]);
	if (w > 0 && h > 0) {
		return tilewindow(argv[1], w,h);
	}
	else {
		printf("width and height must be valid number");
		showhelp();
	}
	return 1;
}

