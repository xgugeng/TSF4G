LIBRARY=lib/libtcommon.a
TARGET= $(LIBRARY)
HOME=../
CINC=-I/usr/local/tlibc/include
TDRINC=-Itdr
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)
WRITER_TDR_FILE=$(wildcard tdr/*.tdr)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
