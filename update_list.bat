@echo off
echo. > list.txt

setlocal enabledelayedexpansion
for /f %%i in ('dir /b /A:D') do (
set NAME=%%i
set FILE=!NAME!/README.md
if exist !FILE! (
echo read !FILE! ...
iconv -f utf-8 -t gbk !FILE! > temp.txt || goto error
set /p TITLE=<temp.txt
set TITLE=!TITLE:#=!
echo - [!NAME!]^(https://github.com/churuxu/codesnippets/tree/master/!NAME!^) !TITLE! >> list.txt
)
)

:error

