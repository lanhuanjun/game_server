#!/bin/bash

function make_logs_dir()
{
    if [ ! -d $1 ];then
        mkdir -p $1
    fi
}

function start_svc()
{
    BIN_PATH=bin
    LOG_DIR="../logs/"
    UPDATE_TICK=1000
    LOG_SETTING="--alsologtostderr=true --log_dir=${LOG_DIR} --colorlogtostderr=true --max_log_size=512 --logbufsecs=1 --vmodule=info=2,error=3"
    EXTEND_PARAM="-SERVICE_UPDATE_TICK=${UPDATE_TICK} ${LOG_SETTING}"

    
    cd $BIN_PATH
    make_logs_dir $LOG_DIR
    echo "start param:${EXTEND_PARAM}"
    ./svc_launch -SERVICE_TYPE=3 -SERVICE_ID=1 $EXTEND_PARAM
    # ./svc_launch -SERVICE_TYPE=1 -SERVICE_ID=1 $EXTEND_PARAM
    # ./svc_launch -SERVICE_TYPE=1 -SERVICE_ID=2 $EXTEND_PARAM
    cd ..
}

start_svc