NAME=tlog
EXECUTABLE=0
DEPTARGET=tcommon tbus tlog
DEPTARET_DIR?=..

CFILE=$(wildcard $(SOURCE)/*.c)
CFILE+=$(wildcard $(SOURCE)/appender/*.c)
SQL_TDATA_FILE = $(INCLUDE)/tlog_sql.td

include ../build.mk
