include ../tdserver.mk

TCONND_CFILE=$(wildcard *.c)
TCONND_OBJ=$(TCONND_CFILE:.c=.o)
TCONND_APP=tconnd


.PHONY: all clean

all: $(TCONND_APP)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(TCONND_APP): $(TCONND_OBJ)
	$(CC) $(STATIC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(TCONND_APP)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(TCONND_APP) $(INSTALL_PREFIX)/bin/

clean:
	$(RM) *.o  *.d
	$(RM) ${TCONND_APP}


