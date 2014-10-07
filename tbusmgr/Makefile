BINARY=bin/tbusmgr
TLIBC=/usr/local/tlibc
TARGET= $(BINARY)
HOME=../
CINC=-Iinclude -I$(HOME)/tbus/include/ -I$(TLIBC)/include
DEPOFILE=$(HOME)/tbus/lib/libtbus.a

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk

dep:
	@mkdir -p bin

install:
	mkdir -p $(PREFIX)
	cp -rf bin $(PREFIX)

