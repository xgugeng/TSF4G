include ../../common.mk

TBUS_CFILE=$(wildcard *.c)
TBUS_OBJ=$(TBUS_CFILE:.c=.o)
TBUS_LIB=$(LIBRARY_PATH)/libtbus.a


.PHONY: all clean

all: $(TBUS_LIB)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(TBUS_LIB): $(TBUS_OBJ)
	ar r $(TBUS_LIB) $^

install:$(TBUS_LIB)
	mkdir -p $(INSTALL_PREFIX)/lib/
	cp $(TBUS_LIB) $(INSTALL_PREFIX)/lib/

tags:
	find /usr/local/tlibc/include/ -name "*.h" | xargs ctags -a --c-types=+p+x 
	find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find ~/TServer/include/ . -name "*.h" | cscope -Rbq

clean:
	$(RM) *.o  *.d
	$(RM) $(TBUS_LIB)
	$(RM) cscope.in.out  cscope.out  cscope.po.out
	$(RM) tags
	


