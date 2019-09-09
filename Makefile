CFLAGS=-Wall -pedantic -std=c89 -O2
#CFLAGS += -static --static

# Needed to remove usleep warning
# (this warning because the compiler is enforcing ANSI C)
CFLAGS += -D_GNU_SOURCE

# Use musl cross compiler

CC=arm-linux-gnueabihf-gcc

default: fbtest knob button input_monitor demo audio

fbtest: fbtest.c
	$(CC) $(CFLAGS) $< -o $@

knob: knob.c
	$(CC) $(CFLAGS) $< -o $@

button: button.c
	$(CC) $(CFLAGS) $< -o $@

input_monitor: input_monitor.c
	$(CC) $(CFLAGS) $< -o $@

demo: demo.c
	$(CC) -Inorns/include $< -o $@ -ljack -lpthread -lm

audio: audio.c
	$(CC) -Inorns/include $< -o $@ -Lnorns/lib -ljack

clean:
	$(RM) fbtest
	$(RM) knob
	$(RM) button
	$(RM) input_monitor
	$(RM) demo
	$(RM) audio

