BINARY=bin/tlibc_example
TARGET= $(BINARY)
HOME=../../
CINC=-I$(HOME)/tlibc/include -I$(HOME)/tlibc/tdr
LDPATH=-lz
DEPOFILE=$(HOME)/tlibc/lib/libtlibc.a
DEPLIBS=

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
