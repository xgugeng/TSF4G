BINARY=bin/tconnd
TARGET= $(BINARY)
HOME=../
CINC=-I/usr/local/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tapp/include/
LDPATH=-L/usr/local/tlibc/lib/
DEPLIBS=-ltlibc
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a
TDRINC=-I$(HOME)/tcommon/tdr/ -I$(HOME)/tlog/tdr/

CFILE=$(wildcard source/*.c)
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p bin

install:
	mkdir -p $(PREFIX)
	cp -rf bin $(PREFIX)
