#!/bin/bash

#ipcs -m | awk '{if ($6==0) print $2}' | xargs -i0 ipcrm -m 0

TBUSMGR=../../tbusmgr/bin/tbusmgr
ipcrm -M 10001
ipcrm -M 10002
${TBUSMGR} -s 10000000 -w 10001
${TBUSMGR} -s 10000000 -w 10002

