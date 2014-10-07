BINARY=bin/tbus_server
TLIBC=/usr/local/tlibc
TARGET= $(BINARY)
HOME=../../../
CINC=-Iinclude -I$(HOME)/tutils/include -I$(HOME)/tutils/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include -I$(TLIBC)/include
LDPATH=-L$(TLIBC)/lib
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a $(HOME)/tutils/lib/libtutils.a
DEPLIBS=-lpthread -ltprotocol

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
