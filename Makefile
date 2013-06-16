# Board settings for avr-libc
MCU = atmega32u4
F_CPU = 16000000
F_CLOCK = $(F_CPU)

# avrdude settings
PROGRAMMER = avr109
# The device or COM port to which the board is connected. This might change
# frequently.
COM = /dev/tty.usbmodem1d11

AVRDUDE = avrdude -p m32u4 -P $(COM) -c $(PROGRAMMER)

# avr-gcc options
# Define F_CPU and F_CLOCK
CDEFS = -DF_CPU=$(F_CPU)UL
CDEFS += -DF_CLOCK=$(F_CLOCK)UL

# compiler optimization level
OPT = s

CFLAGS += $(CDEFS) -g
CFLAGS += -O$(OPT)
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -ffunction-sections
CFLAGS += -fno-inline-small-functions
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -fno-strict-aliasing
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes

# Compiler command
CC = avr-gcc -mmcu=$(MCU) $(CFLAGS)

# Sources and output
OBJDIR = build
SOURCES = $(wildcard *.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))
TARGET = $(OBJDIR)/blinky

# Make a build directory
$(OBJDIR):
	mkdir $(OBJDIR)

# Compile all .c files. Compile only; no linking
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -c -o $@ $<

# Link
$(TARGET).elf: $(OBJECTS)
	$(CC) -o $@ $^

# Create an Intel hex format file for use with avrdude
$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex $^ $@

# Burn the firmwrare to chip.
program: $(TARGET).hex
	$(AVRDUDE) -U flash:w:$<

clean:
	rm -rf $(OBJDIR)
