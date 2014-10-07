BINARY=bin/tconnd_client
TLIBC=/usr/local/tlibc
TARGET= $(BINARY)
HOME=../../../
CINC=-Iinclude -Itdr -I$(HOME)/utils/include -I$(HOME)/tutils/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include -I../tconnd_proto/include/ -I$(TLIBC)/include
LDPATH=-L$(TLIBC)/lib
DEPLIBS=-lpthread -ltprotocol
DEPOFILE=$(HOME)/tlog/lib/libtlog.a $(HOME)/tbusapi/lib/libtbusapi.a $(HOME)/tbus/lib/libtbus.a $(HOME)/tapp/lib/libtapp.a $(HOME)/tutils/lib/libtutils.a

TDRINC=-I$(HOME)/tutils/tdr/
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
