include ../../../common.mk

TBUS_MGR_CFILE=$(wildcard *.c)
TBUS_MGR_OBJ=$(TBUS_MGR_CFILE:.c=.o)
TBUS_MGR=tbus_client

DEPLIBS=-ltbus


.PHONY: all clean

all: $(TBUS_MGR)


%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(TBUS_MGR): $(TBUS_MGR_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(TBUS_MGR)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(TBUS_MGR) $(INSTALL_PREFIX)/bin/

tags:
	find /usr/local/tlibc/include/ -name "*.h" | xargs ctags -a --c-types=+p+x 
	find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find ~/TServer/include/ . -name "*.h" | cscope -Rbq

clean:
	$(RM) *.o  *.d
	$(RM) ${TBUS_MGR}
	$(RM) cscope.in.out  cscope.out  cscope.po.out
	$(RM) tags
