CFLAGS=-Wall -pedantic -std=c89 -O2
CFLAGS += -static --static

# Use musl cross compiler

CROSS-CC=arm-linux-musleabihf-gcc

default: fbtest knob button input_monitor

native: audio

fbtest: fbtest.c
	$(CROSS-CC) $(CFLAGS) $< -o $@

knob: knob.c
	$(CROSS-CC) $(CFLAGS) $< -o $@

button: button.c
	$(CROSS-CC) $(CFLAGS) $< -o $@

input_monitor: input_monitor.c
	$(CROSS-CC) $(CFLAGS) $< -o $@

audio: audio.c
	$(CC) -Inorns/include $< -o $@ -Lnorns/lib -ljack

clean:
	$(RM) fbtest
	$(RM) knob
	$(RM) button
	$(RM) input_monitor

