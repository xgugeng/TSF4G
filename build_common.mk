#parameter list
NAME?=
EXECUTABLE?=0
USE_CXX?=0

DEPTARET_DIR?=..
DEPTARGET?=

DEPLOCALINCLUDE?=/usr/local/TLibC/include
DEPLOCALLD?=/usr/local/TLibC/lib
DEPLOCALLIBS?=tlibc

PREFIX?=/usr/local/$(NAME)

CINC=-I include 
CINC+=$(patsubst %, -I $(DEPTARET_DIR)/%/include, $(DEPTARGET))
CINC+=$(patsubst %, -I %, $(DEPLOCALINCLUDE))
ifdef MAKE_RELEASE
CFLAGS ?= -Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -O3 $(CINC) -DMAKE_RELEASE
CXXFLAGS ?= -Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -O3 $(CINC) -DMAKE_RELEASE
else
CFLAGS ?= -Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -g -ggdb $(CINC) -DMAKE_DEBUG
CXXFLAGS ?= -Wall -Wconversion -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -g -ggdb $(CINC) -DMAKE_DEBUG
endif
#parameter list

LDPATH+=$(patsubst %, -L $(DEPTARET_DIR)/%/lib, $(DEPTARGET))
DEPLIBS+=$(patsubst %, -l %, $(DEPTARGET))

LDPATH+=$(patsubst %, -L %, $(DEPLOCALLD))
DEPLIBS+=$(patsubst %, -l %, $(DEPLOCALLIBS))

SOURCE=source
INCLUDE=include
BINARY=bin
LIBRARY=lib
ETC=etc
TDATA=tdata
THRIFT=thrift

CC = gcc
CXX= g++
AR = ar
RM = /bin/rm -f
INSTALL=cp -rpf




REALCC=$(CC) $(CFLAGS)
REALCXX=$(CXX) $(CXXFLAGS)
ifeq ($(USE_CXX),1)
REALLD=$(CXX) $(LDPATH)
else
REALLD=$(CC) $(LDPATH)
endif
REALAR=$(AR)
REALINSTALL=$(INSTALL)
REALTDATA=$(TDATA) $(CINC)
REALTHRIFT=$(THRIFT) --gen cpp -out $(INCLUDE)

THRIFT_FILES=$(wildcard $(INCLUDE)/*.thrift)
THRIFT_SERVICES_FILES=$(wildcard $(INCLUDE)/*_services.thrift)

THRIFT_CONSTANTS_CCFILE=$(THRIFT_FILES:.thrift=_constants.cpp)
THRIFT_TYPES_CCFILE=$(THRIFT_FILES:.thrift=_types.cpp)
THRIFT_SERVICES_CCFILE=$(THRIFT_SERVICES_FILES:.thrift=.cpp)

THRIFT_CONSTANTS_HFILE=$(THRIFT_FILES:.thrift=_constants.h)
THRIFT_TYPES_HFILE=$(THRIFT_FILES:.thrift=_types.h)
THRIFT_SERVICES_HFILE=$(THRIFT_SERVICES_FILES:.thrift=.h)
THRIFT_SERVICES_SKELETON_CCFILE=$(THRIFT_SERVICES_FILES:.thrift=_async_server.skeleton.cpp)
THRIFT_SERVICES_SKELETON_CCFILE+=$(THRIFT_SERVICES_FILES:.thrift=_server.skeleton.cpp)

THRIFT_OFILE=$(THRIFT_CONSTANTS_CCFILE:.cpp=.o)
THRIFT_OFILE+=$(THRIFT_TYPES_CCFILE:.cpp=.o)
THRIFT_OFILE+=$(THRIFT_SERVICES_CCFILE:.cpp=.o)

TDATA_FILE=$(wildcard $(INCLUDE)/*.td)

TYPES_HFILE=$(patsubst %.td, %_types.h, $(TDATA_FILE))
READER_HFILE=$(TDATA_FILE:.td=_reader.h)
WRITER_HFILE=$(TDATA_FILE:.td=_writer.h)
READER_CFILE=$(patsubst $(INCLUDE)/%.td, $(SOURCE)/%_reader.c, $(TDATA_FILE))
READER_OFILE=$(READER_CFILE:.c=.o)
WRITER_CFILE=$(patsubst $(INCLUDE)/%.td, $(SOURCE)/%_writer.c, $(TDATA_FILE))
WRITER_OFILE=$(WRITER_CFILE:.c=.o)

CFILE=$(wildcard $(SOURCE)/*.c)
CXXFILE=$(wildcard $(SOURCE)/*.cpp)

OFILE=$(CFILE:.c=.o)
OFILE+=$(CXXFILE:.cpp=.o)

LIB=$(LIBRARY)/lib$(NAME).a
APP=$(BINARY)/$(NAME)
ifeq ($(EXECUTABLE),1)
TARGET=$(APP)
else
TARGET=$(LIB)
endif

.PHONY: all clean

all:$(TYPES_HFILE) $(READER_HFILE) $(WRITER_HFILE) $(TARGET)

$(LIB): $(OFILE) $(WRITER_OFILE) $(READER_OFILE) $(THRIFT_OFILE)
	@mkdir -p $(LIBRARY)
	$(REALAR) r $(LIB) $^

$(APP): $(OFILE) $(WRITER_OFILE) $(READER_OFILE) $(THRIFT_OFILE)
	@mkdir -p $(BINARY)
	$(REALLD) -o $@ $^ $(DEPLIBS)

release:
	$(MAKE) all MAKE_RELEASE=1

ifeq ($(USE_CXX),1)
%.o: %.cpp
	$(REALCXX) -o $@ -c $<
else
%.o: %.c
	$(REALCC) -o $@ -c $<
endif

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

%_constants.cpp:%.thrift
	$(REALTHRIFT) $<

%_types.cpp:%.thrift
	$(REALTHRIFT) $<

%_services.cpp:%_services.thrift
	$(REALTHRIFT) $<

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
	@$(RM) $(THRIFT_CONSTANTS_CCFILE) $(THRIFT_TYPES_CCFILE) $(THRIFT_SERVICES_CCFILE)
	@$(RM) $(THRIFT_CONSTANTS_HFILE) $(THRIFT_TYPES_HFILE) $(THRIFT_SERVICES_HFILE)
	@$(RM) $(THRIFT_SERVICES_CCFILE)
	@$(RM) $(THRIFT_SERVICES_HFILE)
	@$(RM) $(THRIFT_SERVICES_SKELETON_CCFILE)

