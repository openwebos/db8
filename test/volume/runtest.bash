#!/bin/bash

# check free space via 'spaceCheck'
TEST_DISK_VOLUME_TEXT=`luna-send -n 1 luna://com.palm.db/internal/spaceCheck '{}'`
USE_SENDER_NAME="com.palm.configurator"
TARGET_DB="com.palm.db"
BYTES_AVL_TEXT=""
DISK_VOLUME=1
ITER_BLOCKS_1K_COUNT=1
ITER_COUNT=10
ITER_ALL=1
ITER_TMP=1
arr_all=$(echo $TEST_DISK_VOLUME_TEXT | tr "," "\n")

for item_all in $arr_all
do
    if [ $ITER_ALL -eq 1 ]; then
        BYTES_AVL_TEXT=$item_all
        #get bytesAvailable value
        arr_tmp=$(echo $BYTES_AVL_TEXT | tr ":" "\n")

        for item_avl in $arr_tmp
        do
          if [ $ITER_TMP -eq 2 ]; then
              DISK_VOLUME=$item_avl
            fi
          ITER_TMP=$(($ITER_TMP+1))
        done
    fi

    ITER_ALL=$(($ITER_ALL+1))
done

echo "Avl space:" $DISK_VOLUME "bytes"

# calc block size to fill = spaceCheck / ($ITER_COUNT * 1024)
ITER_BLOCKS_1K_COUNT=$(($DISK_VOLUME / 1024))
ITER_BLOCKS_1K_COUNT=$(($ITER_BLOCKS_1K_COUNT / $ITER_COUNT))
echo "Number 1K blocks per iteration:" $ITER_BLOCKS_1K_COUNT

# add kind
echo "Add kind"
luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/putKind '{"id":"com.palm.a:1","owner":"'$USE_SENDER_NAME'","indexes":[{"name":"foo","props":[{"name":"foo"}]}]}'

> /tmp/perf_results

# start $ITER_COUNT iterations
for i in $(seq $ITER_COUNT)
do
  echo "ITERATION [$i/$ITER_COUNT]"

  #put data
  ./dbput.bash $TARGET_DB $ITER_BLOCKS_1K_COUNT

  if [ $? -ne 0 ]; then
    echo "Break execution due to error!"
    break
  fi

  # run performance test
  /usr/lib/db8/tests/test_db_performance | tee /tmp/perf_log.$i

  DISK_REPORT=`luna-send -n 1 luna://com.palm.db/internal/spaceCheck '{}'`
  echo "Disk status:" $DISK_REPORT

  #parse time values
  echo "Iteration #"$i >> /tmp/perf_results
  echo "Disk status:" $DISK_REPORT >> /tmp/perf_results
  grep "TOTAL" /tmp/perf_log.$i >> /tmp/perf_results
  echo " "

  #remove temp file
  rm /tmp/perf_log.$i

done

# show result table
echo "Results:"
cat /tmp/perf_results

#delete kind
echo "Delete kind"
luna-send -n 1 -a ${USE_SENDER_NAME} luna://${TARGET_DB}/delKind '{"id":"com.palm.a:1"}'

