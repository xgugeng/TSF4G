include tdserver.mk

WORLDD_CFILE=$(wildcard *.c)
WORLDD_OBJ=$(WORLDD_CFILE:.c=.o)
WORLDD_APP=worldd


.PHONY: all clean

all: $(WORLDD_APP)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(WORLDD_APP): $(WORLDD_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(WORLDD_APP)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(WORLDD_APP) $(INSTALL_PREFIX)/bin/

clean:
	$(RM) *.o  *.d
	$(RM) ${WORLDD_APP}


