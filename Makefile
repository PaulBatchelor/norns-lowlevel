CFLAGS=-Wall -pedantic -std=c89 -O2
CFLAGS += -static --static

# Use musl cross compiler

CC=arm-linux-musleabihf-gcc


default: fbtest knob button

fbtest: fbtest.c
	$(CC) $(CFLAGS) $< -o $@

knob: knob.c
	$(CC) $(CFLAGS) $< -o $@


clean:
	$(RM) fbtest
	$(RM) knob
	$(RM) button

