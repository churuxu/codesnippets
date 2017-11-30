#include <windows.h>
#include <stdio.h>

#include "DragHelper.h"

/*
用于查看当前剪切板内容
*/

int main(int argc, char* argv[]) {
	IDataObject* obj = NULL;
	OleGetClipboard(&obj);
	if (!obj) {
		printf("<NULL>");
		return 0;
	}
	int format = CF_TEXT;	
	if (argc > 1) {
		format = RegisterClipboardFormatA(argv[1]);
	}
	String data;
	if (format == CF_TEXT) {
		data = CDragHelper::GetText(obj);
		
	}
	if (CDragHelper::HasData(obj, format)) {
		data = CDragHelper::GetData(obj, format);		
	}
	if (data.size()) {
		printf("%s", data.c_str());
	}
	else {
		printf("<EMPTY>");
	}
	return 0;
}