include ../../common.mk

NAME=tconnd
#CINC+=
DEPLIBS+=-ltbus -ltlibc -ltcommon

#以下为通用部分
.PHONY: all clean
TDATA_FILE=$(wildcard *.td)

TYPES_HFILE=$(TDATA_FILE:.td=_types.h)
READER_HFILE=$(TDATA_FILE:.td=_reader.h)
WRITER_HFILE=$(TDATA_FILE:.td=_writer.h)
READER_CFILE=$(TDATA_FILE:.td=_reader.c)
WRITER_CFILE=$(TDATA_FILE:.td=_writer.c)

C_FILE=$(wildcard *.c) $(wildcard instance/*.c)
OBJECT_FILE=$(C_FILE:.c=.o) $(READER_CFILE:.c=.o) $(WRITER_CFILE:.c=.o)
LIB=$(LIBRARY_PATH)/lib$(NAME).a
APP=$(NAME)
TARGET=$(APP)

.PHONY: all clean

%_types.h:%.td
	tdata -s $(SOURCE_PATH) -t $(SOURCE_PATH) -gen tlibc_types $(NAME)/$<

%_reader.h:%.td %_types.h
	tdata -s $(SOURCE_PATH) -t $(SOURCE_PATH) -gen tlibc_reader_header $(NAME)/$<

%_writer.h:%.td %_types.h
	tdata -s $(SOURCE_PATH) -t $(SOURCE_PATH) -gen tlibc_writer_header $(NAME)/$<

%_reader.c:%.td %_reader.h %_types.h
	tdata -s $(SOURCE_PATH) -t $(SOURCE_PATH) -gen tlibc_reader $(NAME)/$<
	
%_writer.c:%.td %_writer.h %_types.h
	tdata -s $(SOURCE_PATH) -t $(SOURCE_PATH) -gen tlibc_writer $(NAME)/$<
	

all:$(TYPES_HFILE) $(READER_HFILE) $(WRITER_HFILE) $(TARGET) 

$(LIB): $(OBJECT_FILE)
	ar r $(LIB) $^

$(APP): $(OBJECT_FILE)
	$(CC) -o $@ $^ $(LDPATH) $(DEPLIBS) 

install:
	mkdir -p $(INSTALL_PREFIX)/lib/
	cp $(LIB) $(INSTALL_PREFIX)/lib/

tags:
	find /usr/local/tlibc/include/ -name "*.h" | xargs ctags -a --c-types=+p+x 
	find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find ~/TServer/include/ . -name "*.h" | cscope -Rbq

clean:
	find . -name "*.o" | xargs $(RM)
	find . -name "*.d" | xargs $(RM)
	$(RM) $(TARGET)
	$(RM) $(TYPES_HFILE)
	$(RM) $(READER_HFILE)
	$(RM) $(WRITER_HFILE)
	$(RM) $(READER_CFILE)
	$(RM) $(WRITER_CFILE)
