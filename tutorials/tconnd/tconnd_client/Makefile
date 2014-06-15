BINARY=bin/tconnd_client
TARGET= $(BINARY)
HOME=../../../
CINC=-Iinclude -Itdr -I$(HOME)/tlibc/include -I$(HOME)/tlibc/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include -I../tconnd_proto/include/
LDPATH=
DEPLIBS=-lpthread
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a $(HOME)/tlibc/lib/libtlibc.a

TDRINC=-I$(HOME)/tlibc/tdr/
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
