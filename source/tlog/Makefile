include ../../common.mk

CFILE=$(wildcard *.c)
OBJ=$(CFILE:.c=.o)
LIB=$(LIBRARY_PATH)/libtlog.a
TDFILE=tlog/tlog_config.td
TDCFILE=tlog_config_reader.c
TDHFILE=tlog_config_reader.h tlog_config_types.h
TDTFIE=$(INCLUDE_PATH)/tlog/tlog_config_types.h
TDOBJ=tlog_config_reader.o

.PHONY: all clean

all: $(LIB)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(TDCFILE):
	tdata -s $(INCLUDE_PATH) -t $(INCLUDE_PATH) -gen tlibc_types $(TDFILE)
	tdata -s $(INCLUDE_PATH) -t $(SOURCE_PATH) -gen tlibc_reader_header $(TDFILE)
	tdata -s $(INCLUDE_PATH) -t $(SOURCE_PATH) -gen tlibc_reader $(TDFILE)

$(LIB): $(TDOBJ) $(OBJ)
	ar r $(LIB) $^

install:$(LIB)
	mkdir -p $(INSTALL_PREFIX)/lib/
	cp $(LIB) $(INSTALL_PREFIX)/lib/

tags:
	find /usr/local/tlibc/include/ -name "*.h" | xargs ctags -a --c-types=+p+x 
	find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find ~/TServer/include/ . -name "*.h" | cscope -Rbq

clean:
	$(RM) *.o  *.d
	$(RM) $(LIB)
	$(RM) $(TDHFILE)
	$(RM) $(TDCFILE)
	$(RM) $(TDTFIE)
	$(RM) cscope.in.out  cscope.out  cscope.po.out
	$(RM) tags
