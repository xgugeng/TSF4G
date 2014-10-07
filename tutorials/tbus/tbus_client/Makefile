BINARY=bin/tbus_client
TLIBC=/usr/local/tlibc
TARGET= $(BINARY)
HOME=../../../
CINC=-Iinclude -I$(HOME)/tutils/include -I$(HOME)/tutils/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(TLIBC)/include
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
