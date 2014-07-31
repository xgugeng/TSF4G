BINARY=bin/start-stop-daemon
TARGET= $(BINARY)
HOME=../../

CINC=-Iinclude

CFLAGS=-Wall -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -fPIC

CFILE=$(wildcard source/*.c)

include $(HOME)build.mk

dep:
	@mkdir -p bin

install:
	mkdir -p $(PREFIX)
	cp -rf bin $(PREFIX)
