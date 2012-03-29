# Device makefile
PLATFORM   	:= linux-arm
ARCH_CFLAGS	:= -DMOJ_LINUX -DMOJ_USE_PMLOG $(shell pkg-config --cflags glib-2.0)
BUILD_TYPE := release

LIBS :=  -ljemalloc_mt -lPmLogLib

INCLUDES := -I$(QPEDIR)/include/PmLogLib/IncsPublic -I/usr/include/cjson

include build/Makefile.inc
