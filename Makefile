CFLAGS=-Wall -pedantic -std=c89 -O2

default: fbtest knob

fbtest: fbtest.c
	$(CC) $(CFLAGS) $< -o $@
	    
knob: knob.c
	$(CC) $(CFLAGS) $< -o $@


clean:
	$(RM) fbtest
	$(RM) knob

