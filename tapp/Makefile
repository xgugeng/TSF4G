include tdserver.mk

TAPP_CFILE=$(wildcard *.c)
TAPP_OBJ=$(TAPP_CFILE:.c=.o)
TAPP_LIB=libtapp.a


.PHONY: all clean

all: $(TAPP_LIB)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(TAPP_LIB): $(TAPP_OBJ)
	ar r $(TAPP_LIB) $^

install:$(TAPP_LIB)
	mkdir -p $(INSTALL_PREFIX)/lib/
	cp $(TAPP_LIB) $(INSTALL_PREFIX)/lib/

clean:
	$(RM) *.o  *.d
	$(RM) $(TAPP_LIB)
	


