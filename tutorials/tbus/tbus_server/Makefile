BINARY=bin/tbus_server
TARGET= $(BINARY)
HOME=../../../
CINC=-I/usr/local/tlibc/include/ -Iinclude -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include
LDPATH=-L/usr/local/tlibc/lib
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a
DEPLIBS=-ltlibc

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
