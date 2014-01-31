#!/bin/bash

TBUSMGR=../../package/bin/tbusmgr
ipcrm -M 10001
ipcrm -M 10002
${TBUSMGR} -s 10000000 -w 10001
${TBUSMGR} -s 10000000 -w 10002

