#!/bin/bash

#-----------------------------------------------------------------------------------
# This script will test quota check and subscription on specified db using luna-send
#-----------------------------------------------------------------------------------

if [ $# -eq 0 ]; then
  echo "No arguments supplied. Please use: quota-check-test db_name [sender_name]"
  echo "db_name : com.palm.db | com.palm.testdb | com.palm.mediadb"
  echo "sender_name : by default is com.palm.configurator"
  exit
fi

# init variables
TARGET_DB="com.palm.db"
USE_SENDER_NAME="com.palm.configurator"

if [ $# -gt 0 ]; then
  TARGET_DB=$1
fi

if [ $# -gt 1 ]; then
  USE_SENDER_NAME=$2
fi

#print used vars
echo "Target db:" $TARGET_DB
echo "used sender name:" $USE_SENDER_NAME

LUNA_REPLY_TEXT=""
GREP_OK_TEXT=""
QUOTA_SIZE=2000

echo "- add new kind"
LUNA_REPLY_TEXT=`luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/putKind '{"id":"com.palm.test:1","owner":"'"$USE_SENDER_NAME"'","indexes":[{"name":"foo", "props":[{"name":"foo"}]}]}'`
GREP_OK_TEXT=`echo "'$LUNA_REPLY_TEXT'" | grep "errorCode"`
if [ ${#GREP_OK_TEXT} -ne 0 ]; then
    echo $LUNA_REPLY_TEXT
    return 1
fi

echo "- put quota"
LUNA_REPLY_TEXT=`luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/putQuotas '{"quotas":[{"owner":"'"$USE_SENDER_NAME"'", "size":'${QUOTA_SIZE}'}]}'`
GREP_OK_TEXT=`echo "'$LUNA_REPLY_TEXT'" | grep "errorCode"`
if [ ${#GREP_OK_TEXT} -ne 0 ]; then
    echo $LUNA_REPLY_TEXT
    return 1
fi

echo "- get quota status & subscribe"
LUNA_REPLY_TEXT=`luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/internal/quotaCheck '{"subscribe" : true, "owner":"'"$USE_SENDER_NAME"'"}'`
GREP_OK_TEXT=`echo "'$LUNA_REPLY_TEXT'" | grep "errorCode"`
if [ ${#GREP_OK_TEXT} -ne 0 ]; then
    echo $LUNA_REPLY_TEXT
    return 1
fi
#echo $LUNA_REPLY_TEXT

#get bytesUsed value
BYTES_USED_1=0
ITER_ALL=1
ITER_TMP=1
arr_all=$(echo $LUNA_REPLY_TEXT | tr "," "\n")
for item_all in $arr_all
do
    if [ $ITER_ALL -eq 2 ]; then
        BYTES_AVL_TEXT=$item_all
        arr_tmp=$(echo $BYTES_AVL_TEXT | tr ":" "\n")

        for item_avl in $arr_tmp
        do
          if [ $ITER_TMP -eq 2 ]; then
              BYTES_USED_1=$item_avl
          fi
          ITER_TMP=$(($ITER_TMP+1))
        done
    fi

    ITER_ALL=$(($ITER_ALL+1))
done

echo "- put test data"
LUNA_REPLY_TEXT=`luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/put '{"objects":[{"_kind":"com.palm.test:1","foo":10}]}'`
GREP_OK_TEXT=`echo "'$LUNA_REPLY_TEXT'" | grep "errorCode"`
if [ ${#GREP_OK_TEXT} -ne 0 ]; then
    echo $LUNA_REPLY_TEXT
    return 1
fi

echo "- get quota status again"
LUNA_REPLY_TEXT=`luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/internal/quotaCheck '{"subscribe" : true, "owner":"'"$USE_SENDER_NAME"'"}'`
GREP_OK_TEXT=`echo "'$LUNA_REPLY_TEXT'" | grep "errorCode"`
if [ ${#GREP_OK_TEXT} -ne 0 ]; then
    echo $LUNA_REPLY_TEXT
    return 1
fi
#echo $LUNA_REPLY_TEXT

#get bytesUsed value again
BYTES_USED_2=0
ITER_ALL=1
ITER_TMP=1
arr_all=$(echo $LUNA_REPLY_TEXT | tr "," "\n")
for item_all in $arr_all
do
    if [ $ITER_ALL -eq 2 ]; then
        BYTES_AVL_TEXT=$item_all
        arr_tmp=$(echo $BYTES_AVL_TEXT | tr ":" "\n")

        for item_avl in $arr_tmp
        do
          if [ $ITER_TMP -eq 2 ]; then
              BYTES_USED_2=$item_avl
          fi
          ITER_TMP=$(($ITER_TMP+1))
        done
    fi

    ITER_ALL=$(($ITER_ALL+1))
done

echo $LUNA_REPLY_TEXTecho "- quota unsubscribe"
LUNA_REPLY_TEXT=`luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/internal/quotaCheck '{"subscribe" : false, "owner":"'"$USE_SENDER_NAME"'"}'`
GREP_OK_TEXT=`echo "'$LUNA_REPLY_TEXT'" | grep "errorCode"`
if [ ${#GREP_OK_TEXT} -ne 0 ]; then
    echo $LUNA_REPLY_TEXT
    return 1
fi
#echo $LUNA_REPLY_TEXT

#delete kind
echo "- delete kind"
LUNA_REPLY_TEXT=`luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/delKind '{"id":"com.palm.test:1"}'`

#verify bytesUsed change
if [ $(($BYTES_USED_2 - $BYTES_USED_1)) -ne 0 ]; then
    echo "Success"
    return 0
fi

echo "Failed"
#not a normal exit
return 1
