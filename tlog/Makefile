LIBRARY=lib/libtlog.a
TARGET= $(LIBRARY)
HOME=../
CINC=-Iinclude -Itdr -I$(HOME)/tlibc/include -I$(HOME)/tlibc/tdr/ -I$(HOME)/tbus/include/
TDRINC=-Itdr -I$(HOME)/tlibc/tdr/
CFILE=$(wildcard source/*.c)
CFILE+=$(wildcard source/appender/*.c)

TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
WRITER_TDR_FILE=tdr/tlog_sql.tdr
READER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
	cp -rf tdr $(PREFIX)
	cp -rf sql $(PREFIX)
	find $(PREFIX) -name "*.d" -or -name "*.o" | xargs rm -f
