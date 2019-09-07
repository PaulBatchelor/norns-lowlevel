CFLAGS=-Wall -pedantic -std=c89 -O2
CFLAGS += -static --static

# Needed to remove usleep warning
# (this warning because the compiler is enforcing ANSI C)
CFLAGS += -D_GNU_SOURCE

# Use musl cross compiler

CROSS-CC=arm-linux-musleabihf-gcc

default: fbtest knob button input_monitor

# native stuff must be compiled on norns itself

native: audio demo

fbtest: fbtest.c
	$(CROSS-CC) $(CFLAGS) $< -o $@

knob: knob.c
	$(CROSS-CC) $(CFLAGS) $< -o $@

button: button.c
	$(CROSS-CC) $(CFLAGS) $< -o $@

input_monitor: input_monitor.c
	$(CROSS-CC) $(CFLAGS) $< -o $@

demo: demo.c
	$(CC) -Inorns/include $< -o $@ -Lnorns/lib -ljack -lpthread -lm

audio: audio.c
	$(CC) -Inorns/include $< -o $@ -Lnorns/lib -ljack

clean:
	$(RM) fbtest
	$(RM) knob
	$(RM) button
	$(RM) input_monitor
	$(RM) demo
	$(RM) audio

