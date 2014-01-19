include ../../common.mk

TCONND_CFILE=$(wildcard *.c)
TCONND_OBJ=$(TCONND_CFILE:.c=.o)
TCONND_APP=$(BINRARY_PATH)/tconnd
CONFIG_CFILE=config/config_reader.c
CONFIG_OBJ=config/config_reader.o
DEPLIBS=-ltlibc

CINC+=-I./config

.PHONY: all clean

all: $(TCONND_APP)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(CONFIG_CFILE):config.td
	tdata -s ./ -t config -gen tlibc_types $^
	tdata -s ./ -t config -gen tlibc_reader $^

$(TCONND_APP): $(CONFIG_OBJ) $(TCONND_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(TCONND_APP)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(TCONND_APP) $(INSTALL_PREFIX)/bin/

clean:
	$(RM) *.o  *.d
	$(RM) ${TCONND_APP}
	$(RM) config -rf


