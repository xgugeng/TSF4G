NAME=tlogd
EXECUTABLE=1
DEPTARGET=tcommon tapp tbus tlog
DEPTARET_DIR?=..

DEPLOCALINCLUDE=/usr/local/mysql/include
DEPLOCALLD=/usr/local/mysql/lib
DEPLOCALLIBS=mysqlclient

DEPLOCALINCLUDE+=/usr/local/TLibC/include
DEPLOCALLD+=/usr/local/TLibC/lib
DEPLOCALLIBS+=tlibc

include ../build.mk
