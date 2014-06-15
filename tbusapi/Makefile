LIBRARY=lib/libtbusapi.a
TARGET= $(LIBRARY)
HOME=../
CINC=-I$(HOME)/tlibc/include/ -I$(HOME)/tbus/include/ -Iinclude

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
