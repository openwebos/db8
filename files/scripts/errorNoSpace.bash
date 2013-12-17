#!/bin/bash

# This script is executed by db8 maindb upstart script

#do factory reset
PmLogCtl log DB8 crit "mojodb-luna [] DB8 DBGMSG {} [upstart_maindb] No space left for maindb"
/usr/bin/luna-send -n 1 luna://com.webos.service.tv.systemproperty/doUserDefault '{}