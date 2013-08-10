TDSHOME = .
CC = gcc
RM = /bin/rm -f


# -----------include files -----------------------------------

CINC = -I$(TDSHOME)/ 




#----------------libraris --------------------------
LDPATH += -L/usr/lib/ -L/usr/local/lib   
    

LIBS = -ldl

#C compiler flags
CFLAGS = -g  
CFLAGS += -Wall -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT -fPIC
CFLAGS += $(CINC)


ifdef _RELEASE
CFLAGS += -D __DISABLE_DEBUG_PRINT__
CFLAGS += -D __DISABLE_STAT_DETAIL_PRINT__
CFLAGS += -D NDEBUG
endif
