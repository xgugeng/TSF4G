#!/bin/bash

TBUSMGR=../../tbusmgr/bin/tbusmgr

ipcrm -M 123456
${TBUSMGR} -s 8 -n 1 -w 123456

