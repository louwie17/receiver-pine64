SOURCES = c_gpio.c piHiPri.c wiringPi.c
RCSWITCH_SOURCES = $(SOURCES) RCSwitch.cpp

receiver: receiver.cpp
		g++ -fpermissive -lpthread -pthread -Wno-write-strings -o receiver receiver.cpp $(RCSWITCH_SOURCES)
