FLAGS = -lpthread -pthread

test-rfm: test_rfm69.c
	make -C libs
	cc $(FLAGS) -o test_rfm test_rfm69.c -Llibs/rfm69 -L libs/wiringPi -lrfm69-pine -lwiring-pine

clean:
	make clean -C libs
	rm test_rfm
