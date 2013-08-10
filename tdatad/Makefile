include ../tdserver.mk

TDATAD_CFILE=$(wildcard *.c)
TDATAD_OBJ=$(TDATAD_CFILE:.c=.o)
TDATAD_APP=tdatad


.PHONY: all clean

all: $(TDATAD_APP)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(TDATAD_APP): $(TDATAD_OBJ)
	$(CC) $(STATIC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

clean:
	$(RM) *.o  *.d
	$(RM) ${TDATAD_APP}


