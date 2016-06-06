TARGET      ?= laptop_remote/laptop_remote.ino
ARDUINO     ?= /usr/bin/arduino
SCREEN      ?= /usr/bin/screen
DEVICE      ?= /dev/ttyACM0
BAUDRATE    ?= 115200

all: $(TARGET)
	$(ARDUINO) --verify $(TARGET)

upload: $(TARGET)
	$(ARDUINO) --upload $(TARGET)

debug: upload
	$(SCREEN) $(DEVICE) $(BAUDRATE)

.phony: upload debug
