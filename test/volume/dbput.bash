#!/bin/bash

#------------------------------------------------------------------------
# This script will put test kind and data to specified db using luna-send
#------------------------------------------------------------------------

if [ $# -eq 0 ]; then
  echo "No arguments supplied. Please use: dbput db_name [data_kb] [sender_name]"
  echo "db_name : com.palm.db | com.palm.testdb"
  echo "data_kb : size of data in KB to put, by default is 1"
  echo "sender_name : by default is com.palm.configurator"
  exit
fi

# init variables
TARGET_DB=$1
DATA_BLOCKS_NUMBER=1
USE_SENDER_NAME="com.palm.configurator"

CLR_START="\e[32m"
CLR_END="\e[39m"

if [ $# -gt 2 ]; then
  USE_SENDER_NAME=$3
fi

if [ $# -gt 1 ]; then
  DATA_BLOCKS_NUMBER=$2
fi

#print used vars
echo "Target db:" $TARGET_DB
echo "used sender name:" $USE_SENDER_NAME
echo "data volume to write:" $DATA_BLOCKS_NUMBER "KB"

VALUE_DATA=`cat $(pwd)/data`
LUNA_REPLY_TEXT=""
GREP_OK_TEXT=""

# put data
for i in $(seq $DATA_BLOCKS_NUMBER)
do
  #put 1K
  echo "BLOCK [$i/$DATA_BLOCKS_NUMBER]"

  for x in $(seq 8)
  do
    LUNA_REPLY_TEXT=`luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/batch '{"operations":[{"method":"put","params":{"objects":[{"_kind":"com.palm.a:1","foo":"'"$VALUE_DATA"'"}]}}]}'`
    GREP_OK_TEXT=`echo "'$LUNA_REPLY_TEXT'" | grep "errorCode"`
    if [ ${#GREP_OK_TEXT} -ne 0 ]; then
        echo $LUNA_REPLY_TEXT
        return 1
    fi
  done

done

#echo -e $CLR_START "done" $CLR_END
return 0
