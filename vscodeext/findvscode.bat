@echo off

reg query HKCU\Software\Microsoft\Windows\CurrentVersion\Uninstall /s /f "Visual Studio Code" | find "Software" >> "%TEMP%\\vscodereg.txt"

set /p REGPATH=< "%TEMP%\\vscodereg.txt"

reg query %REGPATH% /v InstallLocation | find "InstallLocation"  >> "%TEMP%\\vscoderegv.txt"

set /p REGV=< "%TEMP%\\vscoderegv.txt"

set REGV=%REGV:InstallLocation=%
set REGV=%REGV:REG_SZ=%
set REGV=%REGV:REG_EXPAND_SZ=%

call :showarg %REGV%
goto error

:showarg
echo %*

:error


