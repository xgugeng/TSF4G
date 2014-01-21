include ../../common.mk

.PHONY: all clean
NAME=tcommon
TDATA_FILE=$(wildcard $(INCLUDE_PATH)/$(NAME)/*.td)
TDATA_FILE_SUFFIX=$(patsubst $(INCLUDE_PATH)/%, %, $(TDATA_FILE))

TYPES_HFILE=$(patsubst %.td, $(INCLUDE_PATH)/%_types.h, $(TDATA_FILE_SUFFIX))
READER_HFILE=$(patsubst %.td, $(INCLUDE_PATH)/%_reader.h, $(TDATA_FILE_SUFFIX))
WRITER_HFILE=$(patsubst %.td, $(INCLUDE_PATH)/%_writer.h, $(TDATA_FILE_SUFFIX))
READER_CFILE=$(patsubst %.td, $(SOURCE_PATH)/%_reader.c, $(TDATA_FILE_SUFFIX))
WRITER_CFILE=$(patsubst %.td, $(SOURCE_PATH)/%_writer.c, $(TDATA_FILE_SUFFIX))

C_FILE=$(wildcard *.c) $(READER_CFILE) $(WRITER_CFILE)

OBJECT_FILE=$(C_FILE:.c=.o)

LIBRARY=$(LIBRARY_PATH)/lib$(NAME).a

.PHONY: all clean

all:$(LIBRARY)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

%.c:
	tdata -s $(INCLUDE_PATH) -t $(INCLUDE_PATH) -gen tlibc_types $(TDATA_FILE_SUFFIX)
	tdata -s $(INCLUDE_PATH) -t $(INCLUDE_PATH) -gen tlibc_reader_header $(TDATA_FILE_SUFFIX)
	tdata -s $(INCLUDE_PATH) -t $(INCLUDE_PATH) -gen tlibc_writer_header $(TDATA_FILE_SUFFIX)
	tdata -s $(INCLUDE_PATH) -t $(SOURCE_PATH) -gen tlibc_reader $(TDATA_FILE_SUFFIX)
	tdata -s $(INCLUDE_PATH) -t $(SOURCE_PATH) -gen tlibc_writer $(TDATA_FILE_SUFFIX)

$(LIBRARY): $(OBJECT_FILE)
	ar r $(LIBRARY) $^

install:
	mkdir -p $(INSTALL_PREFIX)/lib/
	cp $(LIBRARY) $(INSTALL_PREFIX)/lib/

tags:
	find /usr/local/tlibc/include/ -name "*.h" | xargs ctags -a --c-types=+p+x 
	find . -name "*.c" -or -name "*.h" | xargs ctags -a --c-types=+p+x
	find ~/TServer/include/ . -name "*.h" | cscope -Rbq

clean:
	find . -name "*.o" | xargs $(RM)
	find . -name "*.d" | xargs $(RM)
	$(RM) $(TYPES_HFILE)
	$(RM) $(READER_HFILE)
	$(RM) $(WRITER_HFILE)
	$(RM) $(READER_CFILE)
	$(RM) $(WRITER_CFILE)
