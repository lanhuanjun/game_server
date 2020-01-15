#!/bin/sh

function make_logs_dir()
{
    rm -rf $1
    if [ ! -d $1 ];then
        mkdir -p $1
    fi
}

function start_svc()
{
    BIN_PATH=bin
    LOG_DIR="../logs/"
    UPDATE_TICK=50
    LOG_SETTING="--alsologtostderr=false --log_dir=${LOG_DIR} --colorlogtostderr=false --max_log_size=512 --logbufsecs=1 --vmodule=info=2,error=3"
    EXTEND_PARAM="-SERVICE_UPDATE_TICK=${UPDATE_TICK} ${LOG_SETTING}"

    
    cd $BIN_PATH
    make_logs_dir $LOG_DIR
    echo "start param:${EXTEND_PARAM}"
    nohup ./svc_launch -SERVICE_TYPE=3 -SERVICE_ID=1 $EXTEND_PARAM >/dev/null 2>&1 &
    nohup ./svc_launch -SERVICE_TYPE=1 -SERVICE_ID=1 $EXTEND_PARAM >/dev/null 2>&1 &
    nohup ./svc_launch -SERVICE_TYPE=1 -SERVICE_ID=2 $EXTEND_PARAM >/dev/null 2>&1 &
    # nohup ./svc_launch -SERVICE_TYPE=1 -SERVICE_ID=3 $EXTEND_PARAM >/dev/null 2>&1 &
    cd ..
}

start_svc