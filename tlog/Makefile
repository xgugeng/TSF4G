LIBRARY=lib/libtlog.a
TARGET= $(LIBRARY)
HOME=../
CINC=-Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr/ -I$(HOME)/tbus/include/ -I/usr/local/tlibc/include
TDRINC=-Itdr -I$(HOME)/tcommon/tdr/
CFILE=$(wildcard source/*.c)
CFILE+=$(wildcard source/appender/*.c)
DFILE=$(wildcard source/*.d)
DFILE+=$(wildcard source/appender/*.d)
DFILE+=$(wildcard tdr/*.d)

TYPES_TDR_FILE=$(wildcard tdr/*.td)
SQL_TDR_FILE=tdr/tlog_sql.td
WRITER_TDR_FILE=tdr/tlog_sql.td
READER_TDR_FILE=$(wildcard tdr/*.td)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
