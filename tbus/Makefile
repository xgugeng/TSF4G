NAME=tbus
EXECUTABLE=0
PREFIX?=/usr/local/$(NAME)
DEPTARET_DIR=..
DEPTARGET=tcommon tbus tlog

DEPLOCALINCLUDE=/usr/local/TLibC/include
DEPLOCALLD=/usr/local/TLibC/lib
DEPLOCALLIBS=tlibc

CINC=-I include 

CINC+=$(patsubst %, -I %, $(DEPLOCALINCLUDE))
LDPATH=$(patsubst %, -L %, $(DEPLOCALLD))
DEPLIBS=$(patsubst %, -l %, $(DEPLOCALLIBS))

CINC+=$(patsubst %, -I $(DEPTARET_DIR)/%/include, $(DEPTARGET))
LDPATH+=$(patsubst %, -L $(DEPTARET_DIR)/%/lib, $(DEPTARGET))
DEPLIBS+=$(patsubst %, -l %, $(DEPTARGET))

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

ifdef MAKE_RELEASE
CFLAGS = -Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -O3 $(CINC) -DMAKE_RELEASE
else
CFLAGS = -Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -g -ggdb $(CINC) -DMAKE_DEBUG
endif



REALCC=$(CC) $(CFLAGS)
REALLD=$(CC) $(LDPATH)
REALAR=$(AR)
REALINSTALL=$(INSTALL)
REALTDATA=$(TDATA) $(CINC)

TDATA_FILE=$(wildcard $(INCLUDE)/*.td)

TYPES_HFILE=$(patsubst %.td, %_types.h, $(TDATA_FILE))
READER_HFILE=$(TDATA_FILE:.td=_reader.h)
WRITER_HFILE=$(TDATA_FILE:.td=_writer.h)
READER_CFILE=$(patsubst $(INCLUDE)/%.td, $(SOURCE)/%_reader.c, $(TDATA_FILE))
READER_OFILE=$(READER_CFILE:.c=.o)
WRITER_CFILE=$(patsubst $(INCLUDE)/%.td, $(SOURCE)/%_writer.c, $(TDATA_FILE))
WRITER_OFILE=$(WRITER_CFILE:.c=.o)

CFILE=$(wildcard $(SOURCE)/*.c)
OFILE=$(CFILE:.c=.o)

LIB=$(LIBRARY)/lib$(NAME).a
APP=$(BINARY)/$(NAME)
ifeq ($(EXECUTABLE),1)
TARGET=$(APP)
else
TARGET=$(LIB)
endif

.PHONY: all clean

all:$(TYPES_HFILE) $(READER_HFILE) $(WRITER_HFILE) $(TARGET)

$(LIB): $(OFILE) $(WRITER_OFILE) $(READER_OFILE)
	@mkdir -p $(LIBRARY)
	$(REALAR) r $(LIB) $^

$(APP): $(OFILE) $(WRITER_OFILE) $(READER_OFILE)
	@mkdir -p $(BINARY)
	$(REALLD) -o $@ $^ $(DEPLIBS)

release:
	$(MAKE) all MAKE_RELEASE=1

%.o: %.c
	$(REALCC) -o $@ -c $<

$(TYPES_HFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(INCLUDE) -gen tlibc_types $^

$(READER_HFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(INCLUDE) -gen tlibc_reader_header $^

$(WRITER_HFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(INCLUDE) -gen tlibc_writer_header $^

$(READER_CFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(SOURCE) -gen tlibc_reader $^
	
$(WRITER_CFILE):$(TDATA_FILE)
	$(REALTDATA) -o $(SOURCE) -gen tlibc_writer $^


install:
	@mkdir -p $(PREFIX)
ifeq ($(EXECUTABLE),1)
	$(REALINSTALL) $(BINARY) $(PREFIX)
	$(REALINSTALL) $(ETC) $(PREFIX)
else
	$(REALINSTALL) $(INCLUDE) $(PREFIX)
	$(REALINSTALL) $(LIBRARY) $(PREFIX)
endif

tags:
	@find $(DEPLOCALINCLUDE) -name "*.h" | xargs ctags -a --c-types=+p+x 
	@find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	@find . -name "*.h" -or -name "*.c" | cscope -Rbq

clean:
	@find . -name "*.o" | xargs $(RM)
	@$(RM) $(TARGET) $(TYPES_HFILE) $(READER_HFILE) $(WRITER_HFILE) $(READER_CFILE) $(READER_OFILE) $(WRITER_CFILE) $(WRITER_OFILE) tags cscope.in.out cscope.po.out cscope.out

