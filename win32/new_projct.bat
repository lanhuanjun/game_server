REM @echo off
set /p pjt_name=�����빤����:
REM echo on
echo %pjt_name%
echo "��������¹�����Ϊ:%pjt_name%, ������Yȷ��:"
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