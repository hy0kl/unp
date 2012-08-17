#!/bin/bash
#set -x
### please fix next line for your system  ###
abs_path=$(dirname $(pwd))
### !!! ###

# global vars
#start_time=$(date +"%Y-%m-%d:%H:%M:%S")
time_str=$(date +"%Y-%m-%d")
work_path="${abs_path}/ev-sever"
contrib_path="${abs_path}/contrib"
data_path="${work_path}/data"
script="${work_path}/script"

hz2py="${contrib_path}/hz2py"

# 100万内最大的素数
#prime_number=999983
# 1 亿内最大的素数
#prime_number=99999989
prime_number=999983
dict_number=100000

Usage="$0 <runtype:start|stop|make|build|hash|pretreat>"
if [ $# -lt 1 ];
then
    echo "$Usage"
    exit 1
fi
runtype=$1

if [ "$runtype" != "start" ] && [ "$runtype" != "stop" ] &&
    [ "parse" != "$runtype" ] && [ "build" != "$runtype" ] &&
    [ "make" != "$runtype" ] && [ "hash" != "$runtype" ] &&
    [ "pretreat" != "$runtype" ];
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
    php "$script/create_index.php" $number
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

    #if ((argc > 1))
    #then
    #    cd ${hz2py} && make $2
    #else
    #    cd ${hz2py} && make
    #fi

    #ret=$?
    #if ((0 != ret))
    #then
    #    echo "make has some thing wrong for contrib."
    #    cd -
    #    exit -1;
    #else
    #    echo "auto compiled success for contrib!"
    #fi

    #cd -
    #if ((1 == argc))
    #then
    #    cp ${hz2py}/hz2py ${work_path}
    #fi

    exit 0;
fi

# build data
if [ "build" == "$runtype" ]; then
    echo "build index and dict data"
    #create_data $prime_number
    cp "$data_path/original" "$data_path/original.$time_str"
    $work_path/build -s $prime_number

    ret=$?
    if ((0 != ret))
    then
        echo "build data has some thing wrong."
        exit -1;
    else
        echo "build success!"
    fi

    cp "$data_path/index_dict" "$data_path/index_dict.$time_str"
    cp "$data_path/inverted_index" "$data_path/inverted_index.$time_str"

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

#pretreat original
if [ "pretreat" == "$runtype" ]; then
    echo "It do work for pretreatment original."

    tool_hz2py=$work_path/hz2py
    if [ ! -f "$tool_hz2py" ]; then
        echo "NO tools: ${tool_hz2py}"
        exit -1;
    fi

    src_file="${data_path}/original"
    pre_file="${data_path}/pre_orig"
    src_backup="${src_file}.${time_str}"

    : > $pre_file;
    cp "$src_file" "$src_backup"
    php "$script/pretreat.php" "$src_file" "$pre_file"

    ret=$?
    if ((0 == ret))
    then
        cp "$pre_file" "${pre_file}.${time_str}"
        cat "$pre_file" >> "$src_file"
    fi
fi
