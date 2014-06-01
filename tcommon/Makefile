LIBRARY=lib/libtcommon.a
TARGET=dep $(LIBRARY)
HOME=../
CINC=-I/usr/local/tlibc/include
TDRINC=-Itdr
TYPES_TDR_FILE=$(wildcard tdr/*.td)
READER_TDR_FILE=$(wildcard tdr/*.td)
WRITER_TDR_FILE=$(wildcard tdr/*.td)

include $(HOME)/build.mk

dep:
	mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
