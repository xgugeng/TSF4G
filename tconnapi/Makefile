LIBRARY=lib/libtconnapi.a
TARGET= $(LIBRARY)
HOME=../
CINC=-Iinclude -I$(HOME)/tlibc/include -I$(HOME)/tlibc/tdr/ -I$(HOME)/tbusapi/include -I$(HOME)/tbus/include

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
