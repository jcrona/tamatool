GNU_PREFIX =

CC=$(GNU_PREFIX)gcc
LD=$(GNU_PREFIX)ld

LIB_FOLDER = lib

LDLIBS = -lSDL2 -lSDL2_image -lpng
CFLAGS += -Wall -I/usr/include/SDL2/

TARGET = tamatool

LIB_SRCS = $(LIB_FOLDER)/tamalib.c $(LIB_FOLDER)/cpu.c $(LIB_FOLDER)/hw.c

SRCS = tamatool.c program.c image.c state.c mem_edit.c

SRCS += $(LIB_SRCS)
OBJECTS = $(SRCS:.c=.o)


all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo
	@echo -n "Linking ..."
	@$(CC) $(CFLAGS) $(LDFLAGS) $+ -o $@ $(LDLIBS)
	@echo " -> $@"
	@echo

clean:
	$(RM) $(OBJECTS) $(TARGET)

%.o : %.c
	@echo "[$@] ..."
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean
