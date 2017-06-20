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

clean:
	rm receiver transmitter
