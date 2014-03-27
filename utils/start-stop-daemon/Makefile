NAME=start-stop-daemon
EXECUTABLE=1

ifdef MAKE_RELEASE
CFLAGS = -Wall -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -O3 $(CINC) -DMAKE_RELEASE
else
CFLAGS = -Wall -Wcast-qual -Wpointer-arith -Wredundant-decls -Wmissing-declarations -Werror --pipe -g -ggdb $(CINC) -DMAKE_DEBUG
endif
DEPTARET_DIR?=../../

include ../../build.mk
