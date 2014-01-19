include ../../common.mk

.PHONY: all clean
TDFILE=tcommon/tcommon.td
TDOUTS=tcommon/tcommon_types.h

all: 
	tdata -s $(INCLUDE_PATH) -t $(INCLUDE_PATH) -gen tlibc_types $(TDFILE)
	tdata -s $(INCLUDE_PATH) -t $(SOURCE_PATH) -gen tlibc_reader $(TDFILE)

install:

clean:
	$(RM) $(INCLUDE_PATH)/tcommon/tcommon_types.h
	$(RM) $(SOURCE_PATH)/tcommon/tcommon_reader.h
	$(RM) $(SOURCE_PATH)/tcommon/tcommon_reader.c


