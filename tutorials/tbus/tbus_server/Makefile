BINARY=bin/tbus_server
TARGET= $(BINARY)
HOME=../../../
CINC=-I/usr/local/tlibc/include/ -Iinclude -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tbusapi/include -I$(HOME)/tapp/include
LDPATH=-L$(HOME)/tlog/lib/ -L$(HOME)/tbusapi/lib -L$(HOME)/tbus/lib/ -L$(HOME)/tapp/lib -L/usr/local/tlibc/lib
DEPLIBS=-ltlog -ltbusapi -ltbus -ltapp -ltlibc

CFILE=$(wildcard source/*.c)
DFILE=$(wildcard source/*.d)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
