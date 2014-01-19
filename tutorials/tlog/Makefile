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

clean:
	$(RM) *.o  *.d
	$(RM) ${APP}


