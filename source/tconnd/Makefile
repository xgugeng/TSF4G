include ../../common.mk

TCONND_CFILE=$(wildcard *.c)
TCONND_OBJ=$(TCONND_CFILE:.c=.o)
TCONND_APP=$(BINRARY_PATH)/tconnd
CONFIG_CFILE=config/config_reader.c
CONFIG_OBJ=config/config_reader.o
DEPLIBS=-ltlibc -ltlog
TDFILE=tconnd/tconnd_config.td
TDCFILE=tconnd_config_reader.c
TDHFILE=tconnd_config_reader.h tconnd_config_types.h
TDTFIE=$(INCLUDE_PATH)/tconnd/tconnd_config_types.h
TDOBJ=tconnd_config_reader.o

CINC+=-I./config

.PHONY: all clean tags

all: $(TCONND_APP)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(TDCFILE):
	tdata -s $(INCLUDE_PATH) -t $(SOURCE_PATH) -gen tlibc_reader $(TDFILE)
	tdata -s $(INCLUDE_PATH) -t $(INCLUDE_PATH) -gen tlibc_types $(TDFILE)

$(CONFIG_CFILE):config.td
	tdata -s ./ -t config -gen tlibc_types $^
	tdata -s ./ -t config -gen tlibc_reader $^

$(TCONND_APP): $(TDOBJ) $(TCONND_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(TCONND_APP)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(TCONND_APP) $(INSTALL_PREFIX)/bin/

tags:
	find /usr/local/tlibc/include/ -name "*.h" | xargs ctags -a --c-types=+p+x 
	find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find ~/TServer/include/ . -name "*.h" | cscope -Rbq

clean:
	$(RM) *.o  *.d
	$(RM) ${TCONND_APP}
	$(RM) $(TDHFILE)
	$(RM) $(TDCFILE)
	$(RM) $(TDTFIE)
	$(RM) cscope.in.out  cscope.out  cscope.po.out
	$(RM) tags

