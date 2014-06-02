BINARY=bin/start-stop-daemon
TARGET= $(BINARY)
HOME=../../

CINC=-Iinclude

CFLAGS=-Wall -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe

CFILE=$(wildcard source/*.c)
DFILE=$(wildcard source/*.d)

include $(HOME)build.mk

dep:
	@mkdir -p bin

install:
	mkdir -p $(PREFIX)
	cp -rf bin $(PREFIX)
