BINARY=bin/tlogd
TARGET=$(BINARY)
HOME=../
CINC=-I/usr/local/tlibc/include/ -Iinclude -Itdr -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr -I$(HOME)/tlog/include -I$(HOME)/tlog/tdr/ -I$(HOME)/tbus/include/ -I$(HOME)/tapp/include/ -I/usr/local/mysql/include
LDPATH=-L$(HOME)/tlog/lib/ -L$(HOME)/tbus/lib/ -L$(HOME)/tapp/lib/ -L/usr/local/mysql/lib -L/usr/local/tlibc/lib/
DEPLIBS=-ltlog -ltbus -lmysqlclient -ltapp -ltlibc
TDRINC=-I$(HOME)/tcommon/tdr/
CFILE=$(wildcard source/*.c)
TYPES_TDR_FILE=$(wildcard tdr/*.td)
READER_TDR_FILE=$(wildcard tdr/*.td)

include $(HOME)/build.mk
