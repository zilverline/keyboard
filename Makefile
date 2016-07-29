# Board settings for avr-libc
MCU = atmega32u4
F_CPU = 16000000
F_CLOCK = $(F_CPU)
F_USB = $(F_CPU)

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
CDEFS += -DF_USB=$(F_USB)UL

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
CFLAGS += -std=c99

# Compiler command
CC = avr-gcc -mmcu=$(MCU) $(CFLAGS)

# Sources and output
OBJDIR = build
SOURCES = $(wildcard *.c) LUFA/Drivers/USB/Class/Common/HIDParser.c LUFA/Drivers/USB/Class/Device/HIDClassDevice.c LUFA/Drivers/USB/Class/Host/HIDClassHost.c LUFA/Drivers/USB/Core/ConfigDescriptors.c LUFA/Drivers/USB/Core/DeviceStandardReq.c LUFA/Drivers/USB/Core/HostStandardReq.c LUFA/Drivers/USB/Core/Events.c LUFA/Drivers/USB/Core/AVR8/PipeStream_AVR8.c LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_RW.c LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_R.c LUFA/Drivers/USB/Core/AVR8/Template/Template_Pipe_RW.c LUFA/Drivers/USB/Core/AVR8/Template/Template_Endpoint_Control_W.c LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.c LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.c LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c LUFA/Drivers/USB/Core/AVR8/Host_AVR8.c LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c LUFA/Drivers/USB/Core/AVR8/Pipe_AVR8.c LUFA/Drivers/USB/Core/USBTask.c
OUT_DIRS = $(addprefix $(OBJDIR)/,$(SOURCES:%.c=%))
OBJECTS = $(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))
TARGET = $(OBJDIR)/keyboard

# Make a build directory
$(OBJDIR):
	mkdir $(OBJDIR)
	mkdir -p $(OUT_DIRS)

$(SOURCES):
	mkdir $(OUT_DIRS)

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
