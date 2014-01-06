include common.mk

TBUS_MGR_CFILE=$(wildcard *.c)
TBUS_MGR_OBJ=$(TBUS_MGR_CFILE:.c=.o)
TBUS_MGR=tbusmgr


.PHONY: all clean

all: $(TBUS_MGR)

-include $(TBUS_MGR_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(TBUS_MGR): $(TBUS_MGR_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(TBUS_MGR)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(TBUS_MGR) $(INSTALL_PREFIX)/bin/

clean:
	$(RM) *.o  *.d
	$(RM) ${TBUS_MGR}


