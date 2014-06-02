BINARY=bin/tconnd
TARGET= $(BINARY)
HOME=../
CINC=-I/usr/local/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tapp/include/
LDPATH=-L$(HOME)/tlog/lib/ -L$(HOME)/tbus/lib/ -L$(HOME)/tapp/lib/ -L/usr/local/tlibc/lib/
DEPLIBS=-ltlog -ltbus -ltapp -ltlibc
TDRINC=-I$(HOME)/tcommon/tdr/ -I$(HOME)/tlog/tdr/

CFILE=$(wildcard source/*.c)
DFILE=$(wildcard source/*.d)
DFILE+=$(wildcard tdr/*.d)
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p bin

install:
	mkdir -p $(PREFIX)
	cp -rf bin $(PREFIX)
