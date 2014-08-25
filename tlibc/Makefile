LIBRARY=lib/libtlibc.a
TARGET= $(LIBRARY)
HOME=../
CINC=-Iinclude -Isource

TDRINC=-Itdr
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)
WRITER_TDR_FILE=$(wildcard tdr/*.tdr)

CFILE=$(wildcard source/protocol/*.c)
CFILE+=$(wildcard source/core/*.c)

REFILE=$(wildcard source/protocol/*.re)

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
	cp -rf tdr $(PREFIX)
	find $(PREFIX) -name "*.d" -or -name "*.o" | xargs rm -f
