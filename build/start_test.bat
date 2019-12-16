@echo off

set BIN_PATH=bin

cd %BIN_PATH%

set LOG_DIR="../logs/"
set LOG_SET=-alsologtostderr=true -log_dir=%LOG_DIR% -colorlogtostderr=true -max_log_size=512 -logbufsecs=1 -vmodule=info=2,error=3
set UPDATE_TICK=-SERVICE_UPDATE_TICK=50
set EXTEND_PARAM=%UPDATE_TICK% %LOG_SET%

start svc_launch.exe -SERVICE_TYPE=3 -SERVICE_ID=1 %EXTEND_PARAM%
start svc_launch.exe -SERVICE_TYPE=1 -SERVICE_ID=1 %EXTEND_PARAM%
start svc_launch.exe -SERVICE_TYPE=1 -SERVICE_ID=2 %EXTEND_PARAM%
start svc_launch.exe -SERVICE_TYPE=1 -SERVICE_ID=3 %EXTEND_PARAM%

cd ..