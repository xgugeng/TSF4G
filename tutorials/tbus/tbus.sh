#!/bin/bash

TBUSMGR=../../tbusmgr/bin/tbusmgr

ipcrm -M 123456
${TBUSMGR} -s 29 -n 1 -w 123456

