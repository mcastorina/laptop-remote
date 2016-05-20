// This version uses the internal data queing so you can treat it like
// Serial (kinda)!

#include <SPI.h>
#include <Wire.h>
#include "Adafruit_Trellis.h"
#include "Adafruit_BLE_UART.h"

#define DEBUG

/* BLE defines */
// Connect CLK/MISO/MOSI to hardware SPI
// e.g. On UNO & compatible: CLK = 13, MISO = 12, MOSI = 11
#define ADAFRUITBLE_REQ     10
#define ADAFRUITBLE_RDY     2   // This should be an interrupt pin (2 or 3)
#define ADAFRUITBLE_RST     9

/* Trellis defines */
#define NUMTRELLIS          1
#define NUMKEYS             (NUMTRELLIS * 16)
#define INTPIN              A2  // can change
// SDA pin to I2C SDA line (A4)
// SCL pin to I2C SCL line (A5)

Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_TrellisSet trellis = Adafruit_TrellisSet(&matrix0);

Adafruit_BLE_UART BTLEserial =
    Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

// array of times of our first presses of a key
// used for keeping track of when to constantly send
long first_presses[NUMKEYS];
long global_count;

void setup(void) {
#ifdef DEBUG
    Serial.begin(115200);
    while(!Serial);
    Serial.println(F("BLE Trellis Remote Control"));
#endif
    /* BLE setup */
    BTLEserial.begin();
    BTLEserial.setDeviceName("Dennis"); /* 7 characters max! */

    /* Trellis setup */
    // INT pin requires a pullup
    pinMode(INTPIN, INPUT);
    digitalWrite(INTPIN, HIGH);

    // begin() with the addresses of each panel in order
    trellis.begin(0x70);
    trellis.setBrightness(3);   // 15 is max

    // count used for button timing, increases every loop()
    global_count = 0;
}

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;

void loop() {
    // Tell the nRF8001 to do whatever it should be working on
    BTLEserial.pollACI();

    // Ask what is our current status
    aci_evt_opcode_t status = BTLEserial.getState();
    // If the status changed....
    if (status != laststatus) {
        if (status == ACI_EVT_DEVICE_STARTED) {
#ifdef DEBUG
            Serial.println(F("* Advertising started"));
#endif
            ble_disconnected();
        }
        if (status == ACI_EVT_CONNECTED) {
#ifdef DEBUG
            Serial.println(F("* Connected!"));
#endif
            ble_connected();
        }
        if (status == ACI_EVT_DISCONNECTED) {
#ifdef DEBUG
            Serial.println(F("* Disconnected or advertising timed out"));
#endif
            ble_disconnected();
        }
        // Set the last status change to this one
        laststatus = status;
    }

    if (status == ACI_EVT_CONNECTED) {
        // Lets see if there's any data for us!
        if (BTLEserial.available()) {
#ifdef DEBUG
            Serial.print("* ");
            Serial.print(BTLEserial.available());
            Serial.println(F(" bytes available from BTLE"));
#endif
            // Get characters
            while (BTLEserial.available()) {
                char c = BTLEserial.read();
#ifdef DEBUG
                Serial.print(c);
#endif
                // FIXME: use full byte for each
                // lower nibble is address
                // upper nibble is value
                uint8_t led = c & 0x0f;
                uint8_t val = (c & 0xf0) >> 4;

                if (val) trellis.setLED(led);
                else     trellis.clrLED(led);
            }
            trellis.writeDisplay();
        }

        // Go through every button update LEDs
        if (trellis.readSwitches()) {
            for (int i = 0; i < NUMKEYS; i++) {
                if (trellis.isKeyPressed(i))    trellis.setLED(i);
                else                            trellis.clrLED(i);
            }
            trellis.writeDisplay();
        }

        // Go through every button
        for (int i = 0; i < NUMKEYS; i++) {
            if (trellis.isKeyPressed(i)) {
                uint8_t cmd = i;
                if (trellis.justPressed(i)) {
                    first_presses[cmd] = global_count;

                    // write the data
                    BTLEserial.write(&cmd, 1);
#ifdef DEBUG
                    Serial.print(F("* Sending -> \""));
                    Serial.print((char)(cmd + 0x30));
                    Serial.println("\"");
#endif
                }
                else if (first_presses[cmd] < global_count - 20) {
                    // write the data if it has been ~600 ms since first press
                    BTLEserial.write(&cmd, 1);
#ifdef DEBUG
                    Serial.print(F("* Sending -> \""));
                    Serial.print((char)(cmd + 0x30));
                    Serial.println("\"");
#endif
                }
#ifdef DEBUG
                Serial.print(F("* Button pressed -> "));
                if (cmd >= 10) Serial.print((char)((cmd/10) + 0x30));
                Serial.println((char)((cmd%10) + 0x30));
#endif
            }
        }
    }
    global_count++;
    delay(30);  // 30ms delay is required for Trellis
}

void ble_connected(void) {
    // light up all the LEDs in order
    for (int i = 0; i < NUMKEYS; i++)
        trellis.clrLED(i);
    trellis.writeDisplay();

    for (int i = 0; i< NUMKEYS; i++) {
        trellis.setLED(i);
        trellis.writeDisplay();
        delay(50);
    }

    delay(700);
    for (int i = 0; i < NUMKEYS; i++)
        trellis.clrLED(i);
    trellis.writeDisplay();
}
void ble_disconnected(void) {
    // blink all the LEDs
    for (int i = 0; i < NUMKEYS; i++)
        trellis.setLED(i);
    trellis.writeDisplay();

    delay(10);
    for (int i = 0; i < NUMKEYS; i++)
        trellis.clrLED(i);
    trellis.writeDisplay();
}
