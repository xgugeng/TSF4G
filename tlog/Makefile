LIBRARY=lib/libtlog.a
TARGET= $(LIBRARY)
HOME=../
CINC=-Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr/ -I$(HOME)/tbus/include/ -I/usr/local/tlibc/include
TDRINC=-Itdr -I$(HOME)/tcommon/tdr/
CFILE=$(wildcard source/*.c)
CFILE+=$(wildcard source/appender/*.c)

TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
SQL_TDR_FILE=tdr/tlog_sql.tdr
WRITER_TDR_FILE=tdr/tlog_sql.tdr
READER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
