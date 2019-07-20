CFLAGS=-Wall -pedantic -std=c89 -O2

fbtest: fbtest.c
	$(CC) $(CFLAGS) $< -o $@


clean:
	$(RM) fbtest

