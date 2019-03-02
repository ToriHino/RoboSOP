.PHONY: all, clean, fclean, re
HEX2BINDIR = ./										# where is the hex2bin folder : ./ if installed in system
HEX2BIN = hex2bin       							# name of hex2bin command
ASM = sdasz80  										# ASM compilator command name
CC = sdcc  											# SDCC commpilator command name
EMULATOR = ./openMSX/openMSX.app/Contents/MacOS/openmsx -machine Philips_NMS_8255 -ext msxdos2 -diska ./dsk/ # -script emulation/boot.tcl -diskb bin/
C_FILES := $(wildcard *.c) 							# which files to compil 
DEST := dsk/ 										# destination of final files

ASM_FILES := $(wildcard *.s)  						# definition of sources files 
REL_FILES := $(subst .s,.rel,$(ASM_FILES))          # definition of .rel files

IHX_FILES := $(subst .c,.ihx,$(C_FILES))  			# definition of .ihx files 
COM_FILES := $(subst .ihx,.com,$(IHX_FILES)) 		# definition of .com files 

# Folder where to find other compiled files to include
INCLUDEDIR = ./fusion-c/include/
# Folder where to find Fusion.lib 		
LIBDIR = ./fusion-c/lib/

# standrd crt0 
INC1 = $(INCLUDEDIR)crt0_msxdos.rel
# use this crt0 if you want to pass parameters to your program
#INC1 = $(INCLUDEDIR)crt0_msxdos_advanced.rel

#INC2 = $(INCLUDEDIR)
#INC3 = $(INCLUDEDIR)
#INC4 = $(INCLUDEDIR)
#INC5 = $(INCLUDEDIR)
#INC6 = $(INCLUDEDIR)
#INC7 = $(INCLUDEDIR)
#INC8 = $(INCLUDEDIR)
#INC9 = $(INCLUDEDIR)
#INCA = $(INCLUDEDIR)
#INCB = $(INCLUDEDIR)
#INCC = $(INCLUDEDIR)
#INCD = $(INCLUDEDIR)
#INCE = $(INCLUDEDIR)
#INCF = $(INCLUDEDIR)

# Standard Adress code 
ADDR_CODE = 0x107
# use this adress code if you are using crt0_msxdos_advanced
# ADDR_CODE=0x180

ADDR_DATA = 0x0


all:  $(REL_FILES) $(COM_FILES) clean


CCFLAGS = --code-loc $(ADDR_CODE) --data-loc $(ADDR_DATA) --disable-warning 196 -mz80 --no-std-crt0 --opt-code-size fusion.lib -L $(LIBDIR) $(INC1) $(INC2) $(INC3) $(INC4) $(INC5) $(INC6) $(INC7) $(INC8) $(INC9) $(INCA) $(INCB) $(INCC) $(INCD) $(INCE) $(INCF)

%.ihx: %.c
	@SDCC $(CCFLAGS) $^
	@echo "..•̀ᴗ•́)و .. $(CC) is Processing ... !"

%.com: %.ihx
	@hex2bin -e com $^
	@cp $@ $(DEST)
	@echo "... (•̀ᴗ•́)و Updating files in  $(DEST)... !"
%.rel: %.s
#	$(AS) -o $^

clean:
	@rm -f *.com *.asm *.lst *.sym *.bin *.ihx *.lk *.map *.noi *.rel
	@echo "....(╯°□°） temp files removed!"

#emulator:
	$(EMULATOR) &
