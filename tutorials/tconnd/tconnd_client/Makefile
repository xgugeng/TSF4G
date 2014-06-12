BINARY=bin/tconnd_client
TARGET= $(BINARY)
HOME=../../../
CINC=-I/usr/local/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include -I../tconnd_proto/include/
LDPATH=-L/usr/local/tlibc/lib
DEPLIBS=-ltlibc -lpthread
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a

TDRINC=-I$(HOME)/tcommon/tdr/
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
