OUTFILE = buzzer
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c11 -D _POSIX_C_SOURCE=200809L -Wshadow -Werror
OFLAGS = main.o buzzer.o command.o timer.o joystick.o gpio.o

all: app

%.o: %.c 
	$(CC_C) -c $(CFLAGS) $< 

app: $(OFLAGS)
	$(CC_C) $(OFLAGS) -o $(OUTDIR)/$(OUTFILE) -lpthread
	
clean:
	rm $(OUTDIR)/$(OUTFILE) $(OFLAGS)