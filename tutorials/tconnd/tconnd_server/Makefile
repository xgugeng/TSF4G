BINARY=bin/tconnd_server
TARGET= $(BINARY)
HOME=../../../
CINC=-I/usr/local/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include -I$(HOME)/tconnapi/include/ -I../tconnd_proto/include/
LDPATH=-L$(HOME)/tlog/lib/ -L$(HOME)/tbusapi/lib -L$(HOME)/tbus/lib/ -L$(HOME)/tapp/lib -L$(HOME)/tconnapi/lib/ -L/usr/local/tlibc/lib
DEPLIBS=-ltlog -ltapp -ltconnapi -ltbusapi -ltbus -ltlibc

TDRINC=-I$(HOME)/tcommon/tdr/
TYPES_TDR_FILE=$(wildcard tdr/*.td)
READER_TDR_FILE=$(wildcard tdr/*.td)

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
