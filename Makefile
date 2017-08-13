SOURCES = c_gpio.c piHiPri.c wiringPi.c
RCSWITCH_SOURCES = $(SOURCES) RCSwitch.cpp
FLAGS = -fpermissive -lpthread -pthread -Wno-write-strings

receiver: receiver.cpp
		g++ $(FLAGS) -o receiver receiver.cpp $(RCSWITCH_SOURCES)

debug-receiver: receiver.cpp
		g++ -DEBUG $(FLAGS) -o receiver receiver.cpp $(RCSWITCH_SOURCES)

transmitter: transmitter.cpp
		g++ $(FLAGS) -o transmitter transmitter.cpp $(RCSWITCH_SOURCES)

debug-transmitter: transmitter.cpp
		g++ -DEBUG $(FLAGS) -o transmitter transmitter.cpp $(RCSWITCH_SOURCES)

test-rfm: test_rfm69.c
	gcc $(FLAGS) -o test_rfm test_rfm69.c rfm69.c wiringPiSPI.c $(SOURCES)

testrfm: SenderReceiver.c
	g++ $(FLAGS) SenderReceiver.c rfm69.cpp wiringPiSPI.c $(SOURCES) -o SenderReceiver -DRASPBERRY

rfm69: rf69spi.cpp 
	g++ $(FLAGS) rf69spi.cpp wiringPiSPI.c $(SOURCES) -o rf69spi


clean:
	rm receiver transmitter
