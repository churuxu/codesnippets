@echo off

cd /d %~dp0

call findvscode.bat > "%TEMP%\vsp.txt"

set /p VSPATH=< "%TEMP%\vsp.txt"

set VSEXE=%VSPATH%Code.exe

if not exist "%VSEXE%" (
  echo vscode not found 
  goto eof
)


echo vscode:%VSEXE%
set RVSEXE=%VSEXE:\=\\%

set REGF=vscode.reg
echo create %REGF%


echo Windows Registry Editor Version 5.00 > %REGF%
echo. >> %REGF%

echo [HKEY_CLASSES_ROOT\Directory\Background\shell\VSCode] >> %REGF%
echo @="Open With VSCode" >> %REGF%
echo "Icon"="\"%RVSEXE%\",0" >> %REGF%
echo. >> %REGF%
echo [HKEY_CLASSES_ROOT\Directory\Background\shell\VSCode\command] >> %REGF%
echo @="\"%RVSEXE%\" \"%%V\"" >> %REGF%
echo. >> %REGF%

echo [HKEY_CLASSES_ROOT\Directory\shell\VSCode] >> %REGF%
echo @="Open With VSCode" >> %REGF%
echo "Icon"="\"%RVSEXE%\",0" >> %REGF%
echo. >> %REGF%
echo [HKEY_CLASSES_ROOT\Directory\shell\VSCode\command] >> %REGF%
echo @="\"%RVSEXE%\" \"%%V\"" >> %REGF%
echo. >> %REGF%



:eof




