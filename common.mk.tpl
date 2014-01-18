ROOT_DIR=_TSERVER_ROOT_DIR
INSTALL_PREFIX = _TSERVER_INSTALL_PREFIX

BINRARY_PATH=$(ROOT_DIR)/bin
LIBRARY_PATH=$(ROOT_DIR)/lib
INCLUDE_PATH=$(ROOT_DIR)/include

CC = gcc
RM = /bin/rm -f


# -----------include files -----------------------------------

CINC = -I$(INCLUDE_PATH)




#----------------libraris --------------------------
LDPATH += -L$(LIBRARY_PATH)

#C compiler flags

ifdef _RELEASE
CFLAGS = -g -Wall -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT -fPIC $(CINC) -Werror -D NDEBUG -O3
else
CFLAGS = -g -Wall -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT -fPIC $(CINC) -Werror
endif


