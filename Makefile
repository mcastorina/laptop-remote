TARGET      ?= laptop_remote/laptop_remote.ino
ARDUINO     ?= /usr/bin/arduino
SCREEN      ?= /usr/bin/screen
DEVICE      ?= /dev/ttyUSB0
BAUDRATE    ?= 115200

all: $(TARGET)
	$(ARDUINO) --port $(DEVICE) --verify $(TARGET)

upload: $(TARGET)
	$(ARDUINO) --port $(DEVICE) --upload $(TARGET)

debug: upload
	$(SCREEN) $(DEVICE) $(BAUDRATE)

.phony: upload debug
