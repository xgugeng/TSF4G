BINARY=bin/tconnd
TARGET= $(BINARY)
HOME=../
CINC=-I$(HOME)/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tlibc/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tapp/include/
LDPATH=
DEPLIBS=-lrt
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a $(HOME)/tlibc/lib/libtlibc.a
TDRINC=-I$(HOME)/tlibc/tdr/ -I$(HOME)/tlog/tdr/

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
