LIBRARY=lib/libtlibc.a
TARGET= $(LIBRARY)
HOME=../
CINC=-Iinclude -I/usr/local/mysql/include/ -Isource

TDRINC=-Itdr
TYPES_TDR_FILE=$(wildcard tdr/*.tdr)
READER_TDR_FILE=$(wildcard tdr/*.tdr)
WRITER_TDR_FILE=$(wildcard tdr/*.tdr)

CFILE=$(wildcard source/protocol/*.c)
CFILE+=$(wildcard source/core/*.c)

REFILE=$(wildcard source/protocol/*.re)

CFLAGS?=-Wall -Wconversion -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe
#由于MYBIND结构里面的指针缺少const， 所以取消-Wcast-qual编译选项

include $(HOME)/build.mk

dep:
	@mkdir -p lib

install:
	mkdir -p $(PREFIX)
	cp -rf lib $(PREFIX)
	cp -rf include $(PREFIX)
