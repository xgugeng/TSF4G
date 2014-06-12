BINARY=bin/tlogd
TARGET= $(BINARY)
HOME=../
CINC=-I/usr/local/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tapp/include/ -I/usr/local/mysql/include
LDPATH=-L/usr/local/mysql/lib -L/usr/local/tlibc/lib/
DEPLIBS=-lmysqlclient -ltlibc
TDRINC=-I$(HOME)/tcommon/tdr/
DEPOFILE=$(HOME)/tapp/lib/libtapp.a $(HOME)/tlog/lib/libtlog.a $(HOME)/tbus/lib/libtbus.a
CFILE=$(wildcard source/*.c)
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p bin

install:
	mkdir -p $(PREFIX)
	cp -rf bin $(PREFIX)
