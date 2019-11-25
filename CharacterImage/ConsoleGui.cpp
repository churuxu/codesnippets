#include "ConsoleGui.h"
#include "windows.h"



int ConsoleGetWidth(){
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE stdhandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(GetConsoleScreenBufferInfo(stdhandle, &info)){
        return info.dwMaximumWindowSize.X;
    }
    return 0;
}

int ConsoleGetHeight(){
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE stdhandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(GetConsoleScreenBufferInfo(stdhandle, &info)){
        return info.dwMaximumWindowSize.Y;
    }
    return 0;    
}


void ConsoleBackSpace(int count){
    int x,y,w,n,m;
    COORD c;
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE stdhandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(GetConsoleScreenBufferInfo(stdhandle, &info)){
        w = info.dwMaximumWindowSize.X;
        x = info.dwCursorPosition.X;
        y = info.dwCursorPosition.Y;
        n = count % w;
        m = count / w;
        y -= m;
        if(x >= n){
            x -= n;
        }else{
            x = x + w - n;
            y -= 1;
        }
        c.X = x;
        c.Y = y;
        SetConsoleCursorPosition(stdhandle, c);
    }      
}


void ConsoleBackLine(int count){
    int x,y;
    COORD c;
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE stdhandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(GetConsoleScreenBufferInfo(stdhandle, &info)){        
        x = info.dwCursorPosition.X;
        y = info.dwCursorPosition.Y;        
        c.X = 0;
        c.Y = y - count;
        SetConsoleCursorPosition(stdhandle, c);
    }
}


