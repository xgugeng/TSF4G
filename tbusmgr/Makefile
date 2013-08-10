include ../tdserver.mk

TBUS_MGR_CFILE=$(wildcard *.c)
TBUS_MGR_OBJ=$(TBUS_MGR_CFILE:.c=.o)
TBUS_MGR=tbusmgr


.PHONY: all clean

all: $(TBUS_MGR)

-include $(TBUS_MGR_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(TBUS_MGR): $(TBUS_MGR_OBJ)
	$(CC) $(STATIC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

clean:
	$(RM) *.o  *.d
	$(RM) ${TBUS_MGR}


