MAIN           = ELM327SLCAN
SRC            = frontend.c init.c rbuf.c clock.c main.c can.c 
CC             = C:\Program Files\Microchip\xc8\v1.42\bin\xc8.exe
CHIP           = 18F25K80

all: $(MAIN).hex

$(MAIN).hex: $(SRC)
	$(CC) $(SRC) --chip=$(CHIP) --MODE=pro --OPT=+speed --OUTDIR=out -O$(MAIN) --ROM=default,-7cfc-7fff
				 
clean:
	rm -f $(MAIN).hex funclist $(MAIN).cof $(MAIN).hxl $(MAIN).p1 $(MAIN).sdb startup.* $(MAIN).lst $(MAIN).pre $(MAIN).sym

