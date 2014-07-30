BINARY=bin/tbus_server
TARGET= $(BINARY)
HOME=../../../
CINC=-Iinclude -I$(HOME)/tlibc/include -I$(HOME)/tlibc/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include
LDPATH=
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a $(HOME)/tlibc/lib/libtlibc.a
DEPLIBS=-lpthread

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
