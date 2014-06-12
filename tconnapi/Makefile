LIBRARY=lib/libtconnapi.a
TARGET= $(LIBRARY)
HOME=../
CINC=-I/usr/local/tlibc/include/ -Iinclude -I$(HOME)/tcommon/include -I$(HOME)/tcommon/tdr/ -I$(HOME)/tbusapi/include -I$(HOME)/tbus/include

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
