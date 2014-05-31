NAME=tlogd
EXECUTABLE=1
DEPTARGET=tcommon tapp tbus tlog
DEPTARET_DIR?=..

DEPLOCALINCLUDE=/usr/local/mysql/include
DEPLOCALLD=/usr/local/mysql/lib
DEPLOCALLIBS=mysqlclient

DEPLOCALINCLUDE+=/usr/local/tlibc/include
DEPLOCALLD+=/usr/local/tlibc/lib
DEPLOCALLIBS+=tlibc

include ../build.mk
