TARGET = tamatool

LIB_FOLDER = lib
LIB_SRCS = $(LIB_FOLDER)/tamalib.c $(LIB_FOLDER)/cpu.c $(LIB_FOLDER)/hw.c

SRCS = tamatool.c program.c image.c state.c mem_edit.c
SRCS += $(LIB_SRCS)
OBJECTS = $(SRCS:.c=.o)
