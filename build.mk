CC=gcc
AR=ar
RM=/bin/rm -f
INSTALL=cp -rpf
TDR=tdr --MMD

PREFIX?=/usr/local/tsf4g/
SOURCES?=.

CFLAGS?=-Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -fno-common

ifdef debug
DEBUG_CFLAGS=-g3 -ggdb3 -DMAKE_DEBUG
else
DEBUG_CFLAGS=-O3 -DMAKE_RELEASE
endif

REALCC=$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $(CINC)
REALLD=$(CC) $(LDPATH)
REALAR=$(AR)
REALINSTALL=$(INSTALL)
REALTDR=$(TDR) $(TDRINC)

TYPES_HFILE=$(TYPES_TDR_FILE:.tdr=_types.h)
READER_HFILE=$(READER_TDR_FILE:.tdr=_reader.h)
READER_CFILE=$(READER_HFILE:.h=.c)
WRITER_HFILE=$(WRITER_TDR_FILE:.tdr=_writer.h)
WRITER_CFILE=$(WRITER_HFILE:.h=.c)

RE2C_CFILE=$(REFILE:.re=.c)
RE2C_HFILE=$(REFILE:.re=.h)


OFILE=$(CFILE:.c=.o) $(READER_CFILE:.c=.o) $(WRITER_CFILE:.c=.o) $(RE2C_CFILE:.c=.o)
DFILE=$(OFILE:.o=.d) $(TYPES_HFILE:.h=.h.d) $(READER_HFILE:.h=.h.d) $(READER_CFILE:.c=.c.d) $(WRITER_HFILE:.h=.h.d) $(WRITER_CFILE:.c=.c.d)

GENFILE=$(TYPES_HFILE) $(WRITER_HFILE) $(WRITER_CFILE) $(READER_HFILE) $(READER_CFILE) $(RE2C_HFILE) $(RE2C_CFILE)
.PHONY: all clean dep install tags

all:dep $(GENFILE) $(TARGET)

$(LIBRARY): $(OFILE)
	$(REALAR) r $(LIBRARY) $^

$(BINARY): $(OFILE) $(DEPOFILE)
	$(REALLD) -o $@ $^ $(DEPLIBS)

%.o: %.c
	$(REALCC) -o $@ -MMD -c $<

$(TYPES_HFILE):$(TYPES_TDR_FILE)
	$(REALTDR) -g types_h $^

$(READER_HFILE):$(READER_TDR_FILE)
	$(REALTDR) -g reader_h $^

$(READER_CFILE):$(READER_TDR_FILE)
	$(REALTDR) -g reader_c $^
	
$(WRITER_HFILE):$(WRITER_TDR_FILE)
	$(REALTDR) -g writer_h $^

$(WRITER_CFILE):$(WRITER_TDR_FILE)
	$(REALTDR) -g writer_c $^

%.c: %.re
	re2c -c -F -t$*.h -o$*.c $<

%.h: %.re
	re2c -c -F -t$*.h -o$*.c $<

tags:$(GENFILE)
	find $(SOURCES) $^ -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find $(SOURCES) $^ -name "*.h" -or -name "*.c" | cscope -Rbq

clean:
	$(RM) $(TARGET) $(OFILE) $(DFILE) $(GENFILE) tags cscope.in.out cscope.po.out cscope.out

-include $(DFILE)
