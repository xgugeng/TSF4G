LIBRARY=lib/libtutils.a
TLIBC=/usr/local/tlibc/
TARGET= $(LIBRARY)
HOME=../
CINC=-Iinclude -Isource -I$(TLIBC)/include

TDRINC=-Itdr
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)
WRITER_TDR_FILE=$(wildcard tdr/*.tdr)

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
	cp -rf tdr $(PREFIX)
	find $(PREFIX) -name "*.d" -or -name "*.o" | xargs rm -f
