LIBRARY=lib/libtbus.a
TLIBC=/usr/local/tlibc
TARGET= $(LIBRARY)
HOME=../
CINC=-I$(HOME)/tlibc/include/ -Iinclude -I$(TLIBC)/include

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
