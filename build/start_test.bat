@echo off

set BIN_PATH=bin

cd %BIN_PATH%

set LOG_DIR="../logs/"
set LOG_SET=-alsologtostderr=true -log_dir=%LOG_DIR% -colorlogtostderr=true -max_log_size=512 -logbufsecs=1 -vmodule=info=2,error=3
set EXTEND_PARAM= %LOG_SET%

REM start test.exe -port=8000 -thread=4 -peer_ip=127.0.0.1 -peer_port=8001 %EXTEND_PARAM%
start test.exe -port=8001 -thread=4 -peer_ip=127.0.0.1 -peer_port=8000 %EXTEND_PARAM%
cd ..