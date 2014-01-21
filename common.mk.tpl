ROOT_DIR=_ROOT_DIR
INSTALL_PREFIX = _INSTALL_PREFIX

PACKAGE_PATH=$(ROOT_DIR)/package

INCLUDE_PATH=$(PACKAGE_PATH)/include
BINRARY_PATH=$(PACKAGE_PATH)/bin
LIBRARY_PATH=$(PACKAGE_PATH)/lib

SOURCE_PATH=$(ROOT_DIR)/source

CC = gcc
RM = /bin/rm -f


# -----------include files -----------------------------------

CINC = -I$(SOURCE_PATH)
CINC+= -I$(TLIBC_HOME)/include




#----------------libraris --------------------------
LDPATH = -L$(LIBRARY_PATH)
LDPATH+= -L$(TLIBC_HOME)/lib


#C compiler flags

ifdef _RELEASE
CFLAGS = -g -Wall -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT -fPIC $(CINC) -Werror -D NDEBUG -O3
else
CFLAGS = -g -Wall -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT -fPIC $(CINC) -Werror
endif

MAKE = make -I $(ROOT_DIR)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

