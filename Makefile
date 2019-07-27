CFLAGS=-Wall -pedantic -std=c89 -O2
CFLAGS += -static --static

# Use musl cross compiler

CC=arm-linux-musleabihf-gcc


default: fbtest knob button input_monitor

fbtest: fbtest.c
	$(CC) $(CFLAGS) $< -o $@

knob: knob.c
	$(CC) $(CFLAGS) $< -o $@

button: button.c
	$(CC) $(CFLAGS) $< -o $@

input_monitor: input_monitor.c
	$(CC) $(CFLAGS) $< -o $@


clean:
	$(RM) fbtest
	$(RM) knob
	$(RM) button
	$(RM) input_monitor

