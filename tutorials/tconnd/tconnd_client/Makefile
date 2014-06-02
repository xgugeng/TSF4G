BINARY=bin/tconnd_client
TARGET= $(BINARY)
HOME=../../../
CINC=-I/usr/local/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include -I../tconnd_proto/include/
LDPATH=-L$(HOME)/tlog/lib/ -L$(HOME)/tbusapi/lib -L$(HOME)/tbus/lib/ -L$(HOME)/tapp/lib -L/usr/local/tlibc/lib
DEPLIBS=-ltlog -ltbusapi -ltbus -ltapp -ltlibc -lpthread

TDRINC=-I$(HOME)/tcommon/tdr/
TYPES_TDR_FILE=$(wildcard tdr/*.td)
READER_TDR_FILE=$(wildcard tdr/*.td)

CFILE=$(wildcard source/*.c)
DFILE=$(wildcard source/*.d)
DFILE+=$(wildcard tdr/*.d)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
