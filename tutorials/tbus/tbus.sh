#!/bin/bash

TBUSMGR=../../tbusmgr/bin/tbusmgr

ipcrm -M 123456
${TBUSMGR} -s 8 -n 10 -w 123456

