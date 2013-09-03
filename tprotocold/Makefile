include tdserver.mk

TPROTOCOLD_CFILE=$(wildcard *.c)
TPROTOCOLD_OBJ=$(TPROTOCOLD_CFILE:.c=.o)
TPROTOCOLD=tprotocold


.PHONY: all clean

all: $(TPROTOCOLD)

-include $(TPROTOCOLD_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(TPROTOCOLD): $(TPROTOCOLD_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(TPROTOCOLD)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(TPROTOCOLD) $(INSTALL_PREFIX)/bin/

clean:
	$(RM) *.o  *.d
	$(RM) ${TPROTOCOLD}


