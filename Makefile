fbtest: fbtest.c
	$(CC) $< -o $@


clean:
	$(RM) fbtest

