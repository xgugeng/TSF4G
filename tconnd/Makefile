BINARY=bin/tconnd
TARGET= $(BINARY)
TLIBC=/usr/local/tlibc
HOME=../
CINC=-Iinclude -Itdr -I$(HOME)/tutils/include -I$(HOME)/tutils/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tapp/include/ -I$(TLIBC)/include
LDPATH=-L$(TLIBC)/lib
DEPLIBS=-lrt -ltprotocol
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a $(HOME)/tutils/lib/libtutils.a
TDRINC=-I$(HOME)/tutils/tdr/ -I$(HOME)/tlog/tdr/

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
