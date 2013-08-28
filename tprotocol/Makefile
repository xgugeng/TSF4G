include tdserver.mk

TPROTOCOL_CFILE=$(wildcard *.c)
TPROTOCOL_OBJ=$(TPROTOCOL_CFILE:.c=.o)
TPROTOCOL=tprotocol


.PHONY: all clean

all: $(TPROTOCOL)

-include $(TPROTOCOL_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(TPROTOCOL): $(TPROTOCOL_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(TPROTOCOL)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(TPROTOCOL) $(INSTALL_PREFIX)/bin/

clean:
	$(RM) *.o  *.d
	$(RM) ${TPROTOCOL}


