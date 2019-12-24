#!/bin/bash
BIN_PATH=bin

cd $BIN_PATH

LOG_DIR="../logs/"
LOG_SETTING="--alsologtostderr=true --log_dir=${LOG_DIR} --colorlogtostderr=true --max_log_size=512 --logbufsecs=1 --vmodule=info=2,error=3"
UPDATE_TICK=-SERVICE_UPDATE_TICK=1000
EXTEND_PARAM="${UPDATE_TICK} ${LOG_SETTING}"

echo "start param:${EXTEND_PARAM}"
./svc_launch -SERVICE_TYPE=1 -SERVICE_ID=1 $EXTEND_PARAM
cd ..