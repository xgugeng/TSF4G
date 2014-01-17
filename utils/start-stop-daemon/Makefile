include ../../common.mk

CFILE=$(wildcard *.c)
OBJ=$(CFILE:.c=.o)
APP=start-stop-daemon

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

clean:
	$(RM) *.o  *.d
	$(RM) ${APP}


