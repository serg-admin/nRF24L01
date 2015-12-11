programm     = main
mcu          = atmega328p
optimize     = -Os

source_dirs := . tools
#source_dirs := $(addprefix ../, $(source_dirs))

includes           = -I /usr/lib/avr/include/

override compile_flags += -pipe
override CFLAGS        = -g -Wall $(optimize) -mmcu=$(mcu) $(includes)
override LDFLAGS       = -Wl,-Map,$(programm).map

CC             = avr-gcc
OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump
VPATH := $(source_dirs)

search_wildcards := $(addsuffix /*.c, $(source_dirs))

$(programm).hex : $(programm).out $(programm).lst
	$(OBJCOPY) -R .eeprom -O ihex $< $@
$(programm).out : $(notdir $(patsubst %.c,%.o,$(wildcard $(search_wildcards))))
	avr-gcc $(CFLAGS) $(LDFLAGS) $^ -o $@
%.o : %.c
	avr-gcc $(CFLAGS) -c -MD $(addprefix -I,$(source_dirs)) $(compile_flags) $<
$(programm).lst : $(programm).out
	$(OBJDUMP) -h -S $(programm).out > $(programm).lst

include $(wildcard *.d)

.PHONY : clean pro_mini leonardo

clean:
	rm -f *.o *.map *.out *.lst *.hex *.elf *.d
	rm -f tools/*.o

pro_mini: $(programm).hex
	avrdude -c arduino -F -P /dev/ttyUSB0 -p $(mcu) -b 57600  -U flash:w:$(programm).hex:i
leonardo: $(programm).hex
	avrdude -c avr109 -F -P /dev/ttyACM1 -p m32u4 -U flash:w:$(programm).hex:i
