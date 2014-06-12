BINARY=bin/tconnd_server
TARGET= $(BINARY)
HOME=../../../
CINC=-I/usr/local/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include -I$(HOME)/tconnapi/include/ -I../tconnd_proto/include/
LDPATH=-L/usr/local/tlibc/lib
DEPLIBS=-ltlibc
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tapp/lib/libtapp.a $(HOME)/tconnapi/lib/libtconnapi.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a

TDRINC=-I$(HOME)/tcommon/tdr/
TYPES_TDR_FILE=$(wildcard tdr/*.td)
READER_TDR_FILE=$(wildcard tdr/*.td)

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
