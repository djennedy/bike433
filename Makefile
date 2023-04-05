OUTFILE = bike_anti_theft
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf
CC_C = $(CROSS_COMPILE)-gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow -pthread -lm

CFLAGS_NOWORKY = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all:
	$(CC_C) *.c -o $(OUTDIR)/$(OUTFILE) $(CFLAGS)
clean:
	rm $(OUTDIR)/$(OUTFILE)