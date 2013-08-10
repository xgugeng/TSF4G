include ../tdserver.mk

SCENED_CFILE=$(wildcard *.c)
SCENED_OBJ=$(SCENED_CFILE:.c=.o)
SCENED_APP=scened


.PHONY: all clean

all: $(SCENED_APP)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(SCENED_APP): $(SCENED_OBJ)
	$(CC) $(STATIC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(SCENED_APP)

clean:
	$(RM) *.o  *.d
	$(RM) ${SCENED_APP}


