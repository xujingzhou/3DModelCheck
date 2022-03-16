rem clear the temporary directory for VS2015. Added by Johnny Xu,2017/7/19

@echo off 

set curdir=%~dp0
cd /d %curdir%
 
del /q/a/f/s *.db
del /q/a/f/s *.pdb
del /q/a/f/s *.ilk
del /q/a/f/s *.sdf
del /q/a/f/s common.lib Scientree.exe

for /f "delims=" %%i in ('dir /ad /b /s ".vs" "plugins" "x64" "Win32" "ipch" "GeneratedFiles"') do (
	
	rd /s /q "%%i"
	echo deleted "%%i"
)

for /f "delims=" %%i in ('dir /b /s "common\Release" "meshlabplugins\Release" "meshlabplugins\Debug"') do (
	
	del /s /a /q "%%i"
	rd /s /q "%%i"
	echo "%%i"
)

rd /s /q ".vs" "ipch" "common\Release" "common\Debug" "meshlab\Release" "meshlab\Debug"

echo clear work has finished!

pause
