include tdserver.mk

SCENED_CFILE=$(wildcard *.c)
SCENED_OBJ=$(SCENED_CFILE:.c=.o)
SCENED_APP=scened


.PHONY: all clean

all: $(SCENED_APP)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(SCENED_APP): $(SCENED_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(SCENED_APP)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(SCENED_APP) $(INSTALL_PREFIX)/bin/

clean:
	$(RM) *.o  *.d
	$(RM) ${SCENED_APP}


