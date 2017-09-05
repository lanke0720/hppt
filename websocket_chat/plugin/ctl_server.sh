#!/bin/bash

ROOT_PATH=$(pwd)    #相对于output 文件
CONF=$ROOT_PATH/conf/server.conf
BIN=$ROOT_PATH/httpd

server_pid=''
proc=$(basename $0) #显示当前脚本的名称

function usage()
{
    printf "%s [start(-s)|stop(-t)|restart(-rs)]\n" "proc"
}

function is_exist()
{
    name=$(basename $BIN) #获取进程名字
    server_pid=$(pidof $name)#根据进程名字获得进程ID
    if [ $? -eq 0 ];then
        return 0
    else
        return 1
    fi
}

function start_server()
{
    if is_exist;then
        echo "server is exist, pid is : $server_pid"
    else
        ip=$(awk -F: '/^IP/{printf $NF}' $CONF)
        port=$(awk -F: '/^PORT/{printf $NF}' $CONF)

        $BIN $ip $port
        echo "start....done"
    fi
}

function stop_server()
{
    if is_exist; then
        kill -9 $server_pid
        echo "stop ... done"
    else
        echo "server not exist, no need to stop!"
    fi
}

if [ $# -ne 1 ]; then
    usage
    exit 1
fi

case $1 in
    start | -s)
        start_server
    ;;
    stop | -t)
        stop_server
        ;;
    restart | -rs)
        stop_server
        start_server
        ;;
    *)
        usage
        exit 2
        ;;
esac

