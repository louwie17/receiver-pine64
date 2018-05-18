rfm69-lib:
	cc -pthread -Wno-write-strings -Wall -c *.c -L../ -lwiring-pine
	ar -cvq librfm69-pine.a *.o

clean:
	rm *.o
	rm *.a
