#!/bin/sh
#set -x
work_path="/Users/hy0kl/Study/unp/ev-sever"

Usage="$0 <runtype:start|stop|make|build>"
if [ $# -lt 1 ];
then
    echo "$Usage"
    exit 1
fi
runtype=$1

if [ "$runtype" != "start" ] && [ "$runtype" != "stop" ] && [ "parse" != "$runtype" ];
then
    echo "$Usage"
    exit 1
fi

# global functions {
work_pids=""
function get_work_pids()
{
    work_pids=$(ps aux | grep sug-server | grep -v grep | awk '{print $2}' | xargs)
}

cd $work_path

# delete old file for space, befor 30 days.
find "$work_path/data/" -type f -mtime +30 -exec rm {} \;

function create_data()
{
    # create inverted index data and dict data
    php script/create_index.php
    ret=$?
    if ((0 != ret))
    then
        echo "create data has some thing wrong."
        exit -2;
    else
        echo "create data is success!"
    fi
    return 0;
}

# stop process
if [ "stop" == "$runtype" ]; then
    echo "Stop process, please wait a moment. :)"

    get_work_pids
    kill_pids=$work_pids

    #no process need to quit
    if [ "$kill_pids" == "" ]; then
       echo "No process need to quit..."
       exit 0
    fi

    kill -s SIGKILL $kill_pids
    echo "kill SIGKILL: $kill_pids"

    exit 0
fi

# start
if [ "start" == "$runtype" ]; then
    #work_pids=$(ps aux | grep spider.py | grep -v grep | awk '{print $2}' | xargs)
    get_work_pids
    if [ "$work_pids" == "" ]; then
        echo "It is no process working, so let it start to work."
    else
        echo "It has process working now, please stop it before start it."
        exit -1
    fi

    create_data
    $work_path/sug-server -i "$work_path/data/inverted_index" -x "$work_path/data/index_dict" -s 9999991 -a 2000000

    exit 0;
fi

# make
if [ "make" == "$runtype" ]; then
    echo "Let's make"
    make

    ret=$?
    if ((0 != ret))
    then
        echo "make has some thing wrong."
        exit -1;
    else
        echo "auto compiled success!"
    fi

    exit 0;
fi

# build data
if [ "build" == "$runtype" ]; then
    echo "build index and dict data"
    create_data

    ret=$?
    if ((0 != ret))
    then
        echo "build data has some thing wrong."
        exit -1;
    else
        echo "build success!"
    fi

    exit 0;
fi
