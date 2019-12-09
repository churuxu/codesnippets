@echo off

rem 使用gcc编译器计算表达式，例如 gxxcalc 1+2*3



set ARG=%*
set F=%TEMP%\a.cpp
set E=%TEMP%\a.exe

echo #include ^<iostream^> > %F%
echo int main(){ >> %F%
echo     auto v = %ARG%; >> %F%
echo     std::cout^<^<v^<^<std::endl; >> %F%
echo     return 0; >> %F%
echo } >> %F%

g++ -o %E% %F% || goto error

%E%

:error

