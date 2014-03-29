#constant
SOURCE=source
INCLUDE=include
BINARY=bin
LIBRARY=lib
ETC=etc
TDATA=tdata

CC = gcc
AR = ar
RM = /bin/rm -f
INSTALL=cp -rpf

#parameter list
NAME?=
EXECUTABLE?=0

DEPTARET_DIR?=..
DEPTARGET?=

DEPLOCALINCLUDE?=/usr/local/TLibC/include
DEPLOCALLD?=/usr/local/TLibC/lib
DEPLOCALLIBS?=tlibc

PREFIX?=/usr/local/$(NAME)
TDATA_FILE?=$(wildcard $(INCLUDE)/*.td)
CFILE?=$(wildcard $(SOURCE)/*.c)

CINC?=-I $(INCLUDE) 
CINC+=$(patsubst %, -I $(DEPTARET_DIR)/%/include, $(DEPTARGET))
CINC+=$(patsubst %, -I %, $(DEPLOCALINCLUDE))
ifdef MAKE_RELEASE
CFLAGS ?= -Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -O3 $(CINC) -DMAKE_RELEASE
else
CFLAGS ?= -Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -g -ggdb $(CINC) -DMAKE_DEBUG
endif
#parameter list

LDPATH+=$(patsubst %, -L $(DEPTARET_DIR)/%/lib, $(DEPTARGET))
DEPLIBS+=$(patsubst %, -l %, $(DEPTARGET))

LDPATH+=$(patsubst %, -L %, $(DEPLOCALLD))
DEPLIBS+=$(patsubst %, -l %, $(DEPLOCALLIBS))





REALCC=$(CC) $(CFLAGS)
REALLD=$(CC) $(LDPATH)
REALAR=$(AR)
REALINSTALL=$(INSTALL)
REALTDATA=$(TDATA) $(CINC)



TYPES_HFILE=$(patsubst %.td, %_types.h, $(TDATA_FILE))
READER_HFILE=$(TDATA_FILE:.td=_reader.h)
WRITER_HFILE=$(TDATA_FILE:.td=_writer.h)
READER_CFILE=$(patsubst $(INCLUDE)/%.td, $(SOURCE)/%_reader.c, $(TDATA_FILE))
READER_OFILE=$(READER_CFILE:.c=.o)
WRITER_CFILE=$(patsubst $(INCLUDE)/%.td, $(SOURCE)/%_writer.c, $(TDATA_FILE))
WRITER_OFILE=$(WRITER_CFILE:.c=.o)


OFILE=$(CFILE:.c=.o)

LIB=$(LIBRARY)/lib$(NAME).a
APP=$(BINARY)/$(NAME)
ifeq ($(EXECUTABLE),1)
TARGET=$(APP)
else
TARGET=$(LIB)
endif

.PHONY: all clean install tags

all:$(TYPES_HFILE) $(READER_HFILE) $(WRITER_HFILE) $(TARGET)

$(LIB): $(OFILE) $(WRITER_OFILE) $(READER_OFILE) 
	@mkdir -p $(LIBRARY)
	@mkdir -p $(INCLUDE)
	$(REALAR) r $(LIB) $^

$(APP): $(OFILE) $(WRITER_OFILE) $(READER_OFILE)
	@mkdir -p $(BINARY)
	@mkdir -p $(ETC)
	$(REALLD) -o $@ $^ $(DEPLIBS)

release:
	$(MAKE) all MAKE_RELEASE=1

%.o: %.c
	$(REALCC) -o $@ -c $<

$(TYPES_HFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(INCLUDE) -gen types_h $^

$(READER_HFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(INCLUDE) -gen reader_h $^

$(WRITER_HFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(INCLUDE) -gen writer_h $^

$(READER_CFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(SOURCE) -gen reader_c $^
	
$(WRITER_CFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(SOURCE) -gen writer_c $^

install:
	@mkdir -p $(PREFIX)
ifeq ($(EXECUTABLE),1)
	$(REALINSTALL) $(BINARY) $(PREFIX)
	$(REALINSTALL) $(ETC) $(PREFIX)
else
	$(REALINSTALL) $(LIBRARY) $(PREFIX)
	$(REALINSTALL) $(INCLUDE) $(PREFIX)
endif

tags:
	@find $(DEPLOCALINCLUDE) -name "*.h" | xargs ctags -a --c-types=+p+x 
	@find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	@find . -name "*.h" -or -name "*.c" | cscope -Rbq

clean:
	@find . -name "*.o" | xargs $(RM)
	@$(RM) $(TARGET) $(TYPES_HFILE) $(READER_HFILE) $(WRITER_HFILE) $(READER_CFILE) $(READER_OFILE) $(WRITER_CFILE) $(WRITER_OFILE) tags cscope.in.out cscope.po.out cscope.out
