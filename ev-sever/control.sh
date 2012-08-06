#!/bin/bash
#set -x
### please fix next line for your system  ###
abs_path="$HOME/Study/unp"
### !!! ###

work_path="${abs_path}/ev-sever"
contrib_path="${abs_path}/contrib"

hz2py="${contrib_path}/hz2py"

# 1 亿内最大的素数
#prime_number=99999989
prime_number=9999991
dict_number=8000

Usage="$0 <runtype:start|stop|make|build|hash>"
if [ $# -lt 1 ];
then
    echo "$Usage"
    exit 1
fi
runtype=$1

if [ "$runtype" != "start" ] && [ "$runtype" != "stop" ] &&
    [ "parse" != "$runtype" ] && [ "build" != "$runtype" ] &&
    [ "make" != "$runtype" ] && [ "hash" != "$runtype" ];
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
    number=$1
    php script/create_index.php $number
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
    get_work_pids
    if [ "$work_pids" == "" ]; then
        echo "It is no process working, so let it start to work."
    else
        echo "It has process working now, please stop it before start it."
        exit -1
    fi

    $work_path/sug-server -i "$work_path/data/inverted_index" -x "$work_path/data/index_dict" -s $prime_number -a $dict_number

    exit 0;
fi

# make
if [ "make" == "$runtype" ]; then
    argc=$#

    echo "Let's make"
    if ((argc > 1))
    then
        make $2
    else
        make
    fi

    ret=$?
    if ((0 != ret))
    then
        echo "make has some thing wrong."
        exit -1;
    else
        echo "auto compiled success!"
    fi

    if ((argc > 1))
    then
        cd ${hz2py} && make $2
    else
        cd ${hz2py} && make
    fi

    ret=$?
    if ((0 != ret))
    then
        echo "make has some thing wrong for contrib."
        cd -
        exit -1;
    else
        echo "auto compiled success for contrib!"
    fi

    cd -
    if ((1 == argc))
    then
        cp ${hz2py}/hz2py ${work_path}
    fi

    exit 0;
fi

# build data
if [ "build" == "$runtype" ]; then
    echo "build index and dict data"
    create_data $prime_number

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

# check hash value
if [ "hash" == "$runtype" ]; then
    argc=$#
    if [ "$argc" -lt 2 ];
    then
        echo "Need query word"
        echo "Usage: $0 hash <query-word>"
        exit -1
    fi
    query_word=$2
    "$work_path/hash" "$query_word" "$prime_number"
    echo ""

    exit 0;
fi

