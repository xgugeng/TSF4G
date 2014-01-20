include ../../../common.mk

CFILE=$(wildcard *.c)
OBJ=$(CFILE:.c=.o)
APP=$(BINRARY_PATH)/start-stop-daemon

CINC+=-I./


.PHONY: all clean

all: $(APP)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(APP): $(OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:$(APP)
	mkdir -p $(INSTALL_PREFIX)/bin/
	cp $(APP) $(INSTALL_PREFIX)/bin/

tags:
	find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find . -name "*.h" | cscope -Rbq

clean:
	$(RM) *.o  *.d
	$(RM) ${APP}
	$(RM) cscope.in.out  cscope.out  cscope.po.out
	$(RM) tags


