# Name: Makefile
# Author: <insert your name here>
# Copyright: <insert your copyright message here>
# License: <insert your license reference here>

# This is a prototype Makefile. Modify it according to your needs.
# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected.
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

PROJECT    = GyroAcc
MCU        = atxmega128a1
TARGET     = $(PROJECT).elf
#DEVICE     = x128a1
DEVICE     = atxmega128a1
CLOCK      = 32000000

CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -DF_CPU=$(CLOCK) -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT 
# This was part of the file from AVR Studio.. 
#CFLAGS += -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=GyroAcc.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"inc" -I"FWLib/inc"  -I"/usr/local/CrossPack-AVR/avr/include"

## Libraries
LIBS = -lc -lm -lprintf_flt -lscanf_flt 

PROGRAMMER = -c jtag2pdi -P usb
HEX        = default/$(PROJECT).hex
ELF        = default/$(PROJECT).elf

# Source Files
VPATH      = inc:FWLib/src:FWLib/inc
vpath %.h inc
vpath %.h FWLib/inc
vpath *.cpp .
vpath *.cpp FWLib/src

SRC         = GyroAcc.cpp IMU.cpp IMUManager.cpp IMUPacketFifo.cpp 
SRC         += clksystem.cpp GyroCmdProcessor.cpp

FWSRC      = CmdProcessor.cpp HardwareSerial.cpp  DebugPort.cpp
FWSRC      += I2C_Master.cpp NewDel.cpp Print.cpp Port.cpp
FWSRC      += cpp_hacks.cpp fifo.cpp TimerCntr.cpp TimerCore.cpp Time.cpp 

ALLSRC		= $(SRC) $(patsubst %.cpp,FWlib/src/%.cpp,$(FWSRC))

OBJ         = $(patsubst %.cpp,%.o,$(SRC))
FWOBJ       = $(patsubst %.cpp,%.o,$(FWSRC))

OBJECTS    = $(OBJ) $(FWOBJ) 
OBJDIR      = obj
BUILDDIR    = build

# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = $(CC)  -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

all: revision $(TARGET) $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss

depend:
	touch Makefile.dep 
	makedepend -- -D__AVR_ATxmega128A1__ $(CFLAGS) $(INCLUDES) -mmcu=$(DEVICE) -- -fMakefile.dep $(ALLSRC) 

revision:
	python FWLib/UpdateRevision.py

$(OBJECTS) : $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(OBJECTS): %.o : %.cpp
	$(CPP) $(INCLUDES) $(CFLAGS) $(*F).o -c  $<

flash:	all
	$(AVRDUDE) -U flash:w:$(PROJECT).hex:i
	
bflash: all
	$(AVRDUDE) -U flash:w:$(PROJECT).hex:i


# file targets:
##Link
$(TARGET): $(OBJECTS)
	 $(CPP) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

.PHONY : clean revision

clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf dep/* $(PROJECT).hex $(PROJECT).eep
	

include Makefile.dep
