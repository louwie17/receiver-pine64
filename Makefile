SOURCES = c_gpio.c piHiPri.c wiringPi.c
RCSWITCH_SOURCES = $(SOURCES) RCSwitch.cpp
FLAGS = -fpermissive -lpthread -pthread -Wno-write-strings

all: receiverMain transmitterMain
receiverMain: receiverMain.cpp
		g++ $(FLAGS) -o receiverMain receiverMain.cpp $(RCSWITCH_SOURCES) Receiver.cpp

debug-receiverMain: receiverMain.cpp
		g++ -DEBUG $(FLAGS) -o receiverMain receiverMain.cpp $(RCSWITCH_SOURCES) Receiver.cpp

transmitterMain: transmitterMain.cpp
		g++ $(FLAGS) -o transmitterMain transmitterMain.cpp $(RCSWITCH_SOURCES) Transmitter.cpp

debug-transmitterMain: transmitterMain.cpp
		g++ -DEBUG $(FLAGS) -o transmitterMain transmitterMain.cpp $(RCSWITCH_SOURCES) Transmitter.cpp

clean:
	rm receiverMain transmitterMain
