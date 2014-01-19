include ../../common.mk

CFILE=$(wildcard *.c)
OBJ=$(CFILE:.c=.o)
LIB=$(LIBRARY_PATH)/libtlog.a


.PHONY: all clean

all: $(LIB)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(LIB): $(OBJ)
	ar r $(LIB) $^

install:$(LIB)
	mkdir -p $(INSTALL_PREFIX)/lib/
	cp $(LIB) $(INSTALL_PREFIX)/lib/

clean:
	$(RM) *.o  *.d
	$(RM) $(LIB)
	


