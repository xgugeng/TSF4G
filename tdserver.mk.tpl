TDSHOME = .
CC = gcc
RM = /bin/rm -f


# -----------include files -----------------------------------

CINC = -I$(TDSHOME)/ 




#----------------libraris --------------------------
LDPATH += -L/usr/lib/ -L/usr/local/lib   

#C compiler flags

ifdef _RELEASE
CFLAGS = -g -Wall -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT -fPIC $(CINC) -Werror -D NDEBUG -O3
else
CFLAGS = -g -Wall -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT -fPIC $(CINC) -Werror
endif


INSTALL_PREFIX = _INSTALL_PREFIX
