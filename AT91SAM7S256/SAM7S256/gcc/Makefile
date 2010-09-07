BASE = ../..
SRCDIR = $(BASE)/Source
CPUINCDIR = $(BASE)/SAM7S256/Include

TARGET = nxt_firmware

ARM_SOURCES =
THUMB_SOURCES = c_button.c c_cmd.c c_comm.c c_display.c c_input.c c_ioctrl.c \
		c_loader.c c_lowspeed.c c_output.c c_sound.c c_ui.c \
		d_bt.c d_button.c d_display.c d_hispeed.c d_input.c \
		d_ioctrl.c d_loader.c d_lowspeed.c d_output.c d_sound.c \
		d_timer.c d_usb.c \
		m_sched.c \
		errno.c sbrk.c strtod.c sscanf.c \
		Cstartup_SAM7.c

ASM_ARM_SOURCE = Cstartup.S
ASM_THUMB_SOURCE =

vpath %.c $(SRCDIR)
vpath %.c $(CPUINCDIR)
vpath %.c lib
vpath %.S $(CPUINCDIR)

INCLUDES =

MCU = arm7tdmi
STARTOFUSERFLASH_DEFINES = -DSTARTOFUSERFLASH_FROM_LINKER=1
DEFINES = -DPROTOTYPE_PCB_4 -DNEW_MENU -DROM_RUN -DVECTORS_IN_RAM \
	  $(STARTOFUSERFLASH_DEFINES)
OPTIMIZE = -Os -fno-strict-aliasing \
	   -ffunction-sections -fdata-sections
WARNINGS = -Wall -W -Wundef -Wno-unused -Wno-format
THUMB_INTERWORK = -mthumb-interwork
CFLAGS = -mcpu=$(MCU) $(THUMB) $(THUMB_INTERWORK) $(WARNINGS) $(OPTIMIZE)
ASFLAGS = -mcpu=$(MCU) $(THUMB) $(THUMB_INTERWORK)
CPPFLAGS = $(INCLUDES) $(DEFINES) -MMD
LDSCRIPT = nxt.ld
LDFLAGS = -nostdlib -T $(LDSCRIPT) -Wl,--gc-sections
LDLIBS = -lc -lm -lgcc

CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

FWFLASH = fwflash

ARM_OBJECTS = $(ARM_SOURCES:%.c=%.o) $(ASM_ARM_SOURCE:%.S=%.o)
THUMB_OBJECTS = $(THUMB_SOURCES:%.c=%.o) $(THUMB_ARM_SOURCE:%.S=%.o)
OBJECTS = $(ARM_OBJECTS) $(THUMB_OBJECTS)

all: bin

elf: $(TARGET).elf
bin: $(TARGET).bin
sym: $(TARGET).sym
lst: $(TARGET).lst

$(TARGET).elf: THUMB = -mthumb
$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	$(LINK.c) $(OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

%.bin: %.elf
	$(OBJCOPY) --pad-to=0x140000 --gap-fill=0xff -O binary $< $@

%.sym: %.elf
	$(OBJDUMP) -h -t $< > $@

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

$(THUMB_OBJECTS): THUMB = -mthumb

-include $(OBJECTS:%.o=%.d)

program: $(TARGET).bin
	$(FWFLASH) $(TARGET).bin

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).sym $(TARGET).lst \
	$(OBJECTS) $(OBJECTS:%.o=%.d)
