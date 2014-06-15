BINARY=bin/tlogd
TARGET= $(BINARY)
HOME=../
CINC=-Iinclude -Itdr -I$(HOME)/tlibc/include -I$(HOME)/tlibc/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tapp/include/ -I/usr/local/mysql/include
LDPATH=-L/usr/local/mysql/lib
DEPLIBS=-lmysqlclient
TDRINC=-I$(HOME)/tlibc/tdr/
DEPOFILE=$(HOME)/tapp/lib/libtapp.a $(HOME)/tlog/lib/libtlog.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tlibc/lib/libtlibc.a
CFILE=$(wildcard source/*.c)
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p bin

install:
	mkdir -p $(PREFIX)
	cp -rf bin $(PREFIX)
