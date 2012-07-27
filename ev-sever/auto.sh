#!/bin/sh
work_path="/Users/hy0kl/Study/unp/ev-sever"
cd $work_path

make

ret=$?
if ((0 != ret))
then
    echo "make has some thing wrong."
    exit -1;
else
    echo "auto compiled success!"
fi

# delete old file for space, befor 30 days.
find "$work_path/data/" -type f -mtime +30 -exec rm {} \;

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

$work_path/sug-server -i "$work_path/data/inverted_index" -x "$work_path/data/index_dict"
