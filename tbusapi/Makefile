LIBRARY=lib/libtbusapi.a
TARGET=dep $(LIBRARY)
HOME=../
CINC=-I/usr/local/tlibc/include/ -Iinclude -I$(HOME)/tcommon/include -I$(HOME)/tbus/include/

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk

dep:
	mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
