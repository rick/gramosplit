PROG = gramosplit

SRCS = gramofile.c secshms.c signpr_cmf.c signpr_cmf2.c signpr_cmf3.c signpr_copy.c signpr_doubmed.c \
	signpr_exper.c signpr_general.c signpr_main.c signpr_mean.c signpr_median.c signpr_mono.c \
	signpr_rms.c signpr_wav.c tracksplit.c

OBJS = $(SRCS:.c=.o)
SHELL = /bin/sh

CC = gcc
LDFLAGS = 

########## CHOOSE YOUR ARCHITECTURE:

# For Linux (and maybe others), use these:
CFLAGS = -Wall -O4 -funroll-loops -DTURBO_MEDIAN -DTURBO_BUFFER
DEPS = $(OBJS)
LIBS = -lrfftw -lfftw -lm
COPY_A = -a

# For FreeBSD (and maybe others), use these:
#CFLAGS = -Wall -O2 -DTURBO_MEDIAN -DTURBO_BUFFER
#DEPS = $(OBJS)
#LIBS = -lrfftw -lfftw -lm
#COPY_A = -p

# For IRIX (and maybe others), use these:
#CFLAGS = -Wall -O2 -DTURBO_MEDIAN -DTURBO_BUFFER -DSWAP_ENDIAN -DOLD_CURSES
#DEPS = $(OBJS)
#LIBS = -lrfftw -lfftw -lm
#COPY_A = -a

##########


$(PROG): $(DEPS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROG) $(LIBS)

.PHONY: clean
clean:
	-rm -f gramosplit *.o *.d *~

.PHONY: indent
indent:
	indent *.c *.h

#%.d: %.c   - according to 'info make', doesn't work
#	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $< \
#		      | sed '\''s/\($*\)\.o[ :]*/\1 $@/g'\'' > $@'
#
# 'some.o: some.c other.h'   ==> 'some some.dsome.c other.h'

%.d: %.c
	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $< \
		      | sed '\''s/\($*\)\.o/& $@/g'\'' > $@'
#
# 'some.o: some.c other.h'   ==> 'some.o some.d: some.c other.h'  => OK

include $(SRCS:.c=.d)

