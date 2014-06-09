rem this batch creates NSIS installer for trace_server in OUTDIR
@echo off
set PATH="C:\Program Files\WinRAR\";%path%
set OUTDIR=c:\_builds\trace_server

echo copying NSIS installer files...

robocopy.exe ..\installer %OUTDIR% trace_server.nsi installer.bmp 
robocopy.exe ..\trace_server\images %OUTDIR% Icon.ico
robocopy.exe .. %OUTDIR% LICENSE
robocopy.exe ..\utils\redistributables\2012u4 %OUTDIR%\redist vcredist_x86.exe vcredist_x64.exe
rem if %errorlevel% neq 0 goto TERM

echo NSIS creating installer.exe ...

set PATH=C:\Program Files (x86)\NSIS\unicode;%PATH%

set OLDDIR=%CD%
chdir /d %OUTDIR%
makensis.exe trace_server.nsi
rem makensis.exe /X"SetCompressor /FINAL lzma" myscript.nsi
if %errorlevel% neq 0 goto TERM

goto NOPAUSE

:TERM
pause

:NOPAUSE

chdir /d %OLDDIR%
