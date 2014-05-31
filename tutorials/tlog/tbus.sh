#!/bin/bash

#ipcs -m | awk '{if ($6==0) print $2}' | xargs -i0 ipcrm -m 0

TBUSMGR=../../tbusmgr/bin/tbusmgr
ipcrm -M 20001
${TBUSMGR} -s65535 -n1024 -w 20001

