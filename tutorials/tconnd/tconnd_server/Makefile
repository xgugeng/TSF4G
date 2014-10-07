BINARY=bin/tconnd_server
TLIBC=/usr/local/tlibc
TARGET= $(BINARY)
HOME=../../../
CINC=-Iinclude -Itdr -I$(HOME)/tutils/include -I$(HOME)/tutils/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include -I$(HOME)/tconnapi/include/ -I../tconnd_proto/include/ -I$(TLIBC)/include
LDPATH=-L$(TLIBC)/lib
DEPLIBS=-lpthread -ltprotocol
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tapp/lib/libtapp.a $(HOME)/tconnapi/lib/libtconnapi.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tutils/lib/libtutils.a

TDRINC=-I$(HOME)/tutils/tdr/
TYPES_TDR_FILE=$(wildcard tdr/*.td)
READER_TDR_FILE=$(wildcard tdr/*.td)

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
