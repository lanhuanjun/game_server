REM @echo off
set /p pjt_name=请输入工程名:
REM echo on
echo %pjt_name%
echo "您输入的新工程名为:%pjt_name%, 请输入Y确认:"
set /p ok=

IF NOT "%ok%" == "Y" (
	pause;
	exit;
)
REM @echo off
cd projects
cp demo.vcxproj %pjt_name%.vcxproj
cp demo.vcxproj.filters %pjt_name%.vcxproj.filters
cd ..
cd ..\src\
pause