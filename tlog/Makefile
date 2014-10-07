LIBRARY=lib/libtlog.a
TLIBC=/usr/local/tlibc
TARGET= $(LIBRARY)
HOME=../
CINC=-Iinclude -Itdr -I$(HOME)/tutils/include -I$(HOME)/tutils/tdr/ -I$(HOME)/tbus/include/ -I$(TLIBC)/include
TDRINC=-Itdr -I$(HOME)/tutils/tdr/
CFILE=$(wildcard source/*.c)
CFILE+=$(wildcard source/appender/*.c)

TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
WRITER_TDR_FILE=tdr/tlog_message.tdr
READER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
	cp -rf tdr $(PREFIX)
	find $(PREFIX) -name "*.d" -or -name "*.o" | xargs rm -f
