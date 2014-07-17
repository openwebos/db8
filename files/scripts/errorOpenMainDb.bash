#! /bin/bash
# Ignore IO error if either of condtions are true
#   - '/var/db' is mounted as read-only.
#   - shutdown/reboot is ongoing.
cat /proc/mounts | grep '/var/db ' |  grep -qw 'ro' && exit

# checks whether reboot/shutdown is ongoing
initctl status shutdown | grep -q running && touch /tmp/shutdown_running; initctl status reboot | grep -q running && exit

# check available disk space
BYTES_FREE=$(($(stat -f -c "%a*%S" /mnt/lg/cmn_data)))
BYTES_THRESHOLD=$((100*1024*1024))  # 100M will be enough for database backup

# if enough disk space, backup current database for feature analisys
if [ "$BYTES_FREE" -ge "$BYTES_THRESHOLD" ]; then
    mkdir -p /mnt/lg/cmn_data/db8
    BACKUP_FILENAME="/mnt/lg/cmn_data/db8/corrupted_maindb.tar.bz2"
    rm -f $BACKUP_FILENAME
    tar cjf $BACKUP_FILENAME /var/db/main
    chmod 400 $BACKUP_FILENAME
    rm -rf /var/db/main/*
else
    PmLogCtl log DB8 crit "mojodb-luna [] DB8 DBGMSG {} [maindb_errorOpenmainDb.bash] No space left to store corrupted maindb"
fi

#do factory reset
/usr/bin/luna-send -n 1 luna://com.webos.service.tv.systemproperty/doUserDefault '{}'
