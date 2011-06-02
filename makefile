BIN       = build
KERNEL    = kernel
LIB		  = lib
DRV       = drivers
ARCH      = arch
USER	  = usr
INC		  = include

IMAGE     = vm/kernel.img
KERNELELF = build/kernel.elf
MAPFILE   = build/eos.map

LINKFILE  = eos.ld

CFLAGS    = -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I$(INC) -Wall -O -g -c
LDFLAGS   = --cref -Map $(MAPFILE) -T $(LINKFILE)

################################################################################
# Cross Compiler
################################################################################
CC = /usr/local/cross/bin/i586-elf-gcc
LD = /usr/local/cross/bin/i586-elf-ld

################################################################################
# Objects
################################################################################
OBJFILES := $(OBJFILES) $(patsubst $(KERNEL)/%.c,$(BIN)/%.o,$(wildcard $(KERNEL)/*.c))
OBJFILES := $(OBJFILES) $(patsubst $(LIB)/%.c,$(BIN)/%.o,$(wildcard $(LIB)/*.c))
OBJFILES := $(OBJFILES) $(patsubst $(DRV)/%.c,$(BIN)/%.o,$(wildcard $(DRV)/*.c))
OBJFILES := $(OBJFILES) $(patsubst $(ARCH)/%.c,$(BIN)/%.o,$(wildcard $(ARCH)/*.c))
OBJFILES := $(OBJFILES) $(patsubst $(ARCH)/%.asm,$(BIN)/%.o,$(wildcard $(ARCH)/*.asm))
OBJFILES := $(OBJFILES) $(patsubst $(USER)/%.c,$(BIN)/%.o,$(wildcard $(USER)/*.c))

################################################################################
# Headers
################################################################################
DEPENDENTS := build.bat makefile $(LINKFILE)
DEPENDENTS := $(DEPENDENTS) $(wildcard $(INC)/*.h)
DEPENDENTS := $(DEPENDENTS) $(wildcard $(INC)/$(KERNEL)/*.h)
DEPENDENTS := $(DEPENDENTS) $(wildcard $(ARCH)/*.h)
DEPENDENTS := $(DEPENDENTS) $(wildcard $(DRV)/*.h)
DEPENDENTS := $(DEPENDENTS) $(wildcard $(USER)/*.h)

################################################################################
# All / Kernel
################################################################################
all: $(KERNELELF)
$(KERNELELF): $(OBJFILES)
	@$(LD) $^ $(LDFLAGS) -o $@

################################################################################
# Arch
################################################################################
$(BIN)/%.o : $(ARCH)/%.c $(DEPENDENTS)
	@$(CC) $(CFLAGS) $< -o $@
$(BIN)/%.o : $(ARCH)/%.asm $(DEPENDENTS)
	@nasm -f elf32 -o $@ $<

################################################################################
# Kernel
################################################################################
$(BIN)/%.o : $(KERNEL)/%.c $(DEPENDENTS)
	@$(CC) $(CFLAGS) $< -o $@

################################################################################
# Drivers
################################################################################
$(BIN)/%.o : $(DRV)/%.c $(DEPENDENTS)
	@$(CC) $(CFLAGS) $< -o $@

################################################################################
# C Library
################################################################################
$(BIN)/%.o : $(LIB)/%.c $(DEPENDENTS)
	@$(CC) $(CFLAGS) $< -o $@

################################################################################
# User Programs
################################################################################
$(BIN)/%.o : $(USER)/%.c $(DEPENDENTS)
	@$(CC) $(CFLAGS) $< -o $@


clean:
	rm $(BIN)/*.o
	rm $(MAPFILE)
	rm $(KERNELELF)
