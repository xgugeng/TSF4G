include ../tdserver.mk

TBUS_CFILE=$(wildcard *.c)
TBUS_OBJ=$(TBUS_CFILE:.c=.o)
TBUS_LIB=libtbus.a

TBUS_LIBS= -lpthread

ifeq ($(release), 1)
	STATIC = -static
endif
    

.PHONY: all clean

all: $(TBUS_LIB)

-include $(TBUS_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(TBUS_LIB): $(TBUS_OBJ)
	ar r $(TBUS_LIB) $^

clean:
	$(RM) *.o  *.d
	$(RM) $(TBUS_LIB)
	


