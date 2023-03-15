OUTFILE = bike_anti_theft
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf
CC_C = $(CROSS_COMPILE)-gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow -pthread

CFLAGS_NOWORKY = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all:
	$(CC_C) $(CFLAGS) *.c -o $(OUTDIR)/$(OUTFILE)
clean:
	rm $(OUTDIR)/$(OUTFILE)