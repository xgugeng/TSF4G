BINARY=bin/tutils_example
TLIBC=/usr/local/tlibc
TARGET= $(BINARY)
HOME=../../
CINC=-I$(HOME)/tutils/include -I$(HOME)/tutils/tdr -I$(TLIBC)/include
LDPATH=-lz
DEPOFILE=$(HOME)/tutils/lib/libtutils.a
DEPLIBS=

CFILE=$(wildcard source/*.c)

include $(HOME)/build.mk
dep:
	@mkdir -p bin
