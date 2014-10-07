BINARY=bin/tlogd
TLIBC=/usr/local/tlibc
TARGET= $(BINARY)
HOME=../
CINC=-Iinclude -Itdr -I$(HOME)/tutils/include -I$(HOME)/tutils/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbusapi/include/ -I$(HOME)/tbus/include/ -I$(HOME)/tapp/include/ -I$(TLIBC)/include
LDPATH=-L$(TLIBC)/lib
DEPLIBS=-lpthread -ltprotocol
TDRINC=-I$(HOME)/tutils/tdr/ -I$(HOME)/tlog/tdr/
DEPOFILE=$(HOME)/tapp/lib/libtapp.a $(HOME)/tlog/lib/libtlog.a  $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tutils/lib/libtutils.a
CFILE=$(wildcard source/*.c)
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p bin

install:
	mkdir -p $(PREFIX)
	cp -rf bin $(PREFIX)
	cp -rf tdr $(PREFIX)
	find $(PREFIX) -name "*.d" -or -name "*.o" | xargs rm -f
