PLATFORM   	:= mac-x86
ARCH_CFLAGS	:= -DMOJ_MAC -DMOJ_X86 $(shell pkg-config --cflags glib-2.0)
ICU_LDFLAGS := -licui18n -licuuc -licudata
ARCH_LDFLAGS := -L$(LUNA_STAGING)/lib $(shell pkg-config --libs glib-2.0) $(ICU_LDFLAGS)

INCLUDES := \
	-I$(LUNA_STAGING)/include \
	$(INCLUDES)

include build/Makefile.inc
