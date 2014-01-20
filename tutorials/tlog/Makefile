include ../../common.mk

CFILE=$(wildcard *.c)
OBJ=$(CFILE:.c=.o)
APP=tlog_example

DEPLIBS=-ltlog -ltlibc


.PHONY: all clean

all: $(APP)


%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(APP): $(OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

tags:
	find /usr/local/tlibc/include/ -name "*.h" | xargs ctags -a --c-types=+p+x 
	find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find ~/TServer/include/ . -name "*.h" | cscope -Rbq

clean:
	$(RM) *.o  *.d
	$(RM) ${APP}
	$(RM) cscope.in.out  cscope.out  cscope.po.out
	$(RM) tags
