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
    trellis.begin(0x70);  // only one
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
#ifdef DEBUG
        // Lets see if there's any data for us!
        if (BTLEserial.available()) {
            Serial.print("* ");
            Serial.print(BTLEserial.available());
            Serial.println(F(" bytes available from BTLE"));
        }
#endif
        // Get characters
        while (BTLEserial.available()) {
            char c = BTLEserial.read();
#ifdef DEBUG
            Serial.print(c);
#endif
        }

        // FIXME: change to use buttons
        if (trellis.readSwitches()) {
            uint8_t cmd = 0xff;
            // Go through every button
            for (int i = 0; i < NUMKEYS; i++) {
                if (trellis.justPressed(i)) {
                    cmd = i;
#ifdef DEBUG
                    Serial.print(F("* Button pressed -> "));
                    if (cmd >= 10) Serial.print((char)((cmd/10) + 0x30));
                    Serial.println((char)((cmd%10) + 0x30));
#endif
                    if (trellis.isLED(cmd)) trellis.clrLED(cmd);
                    else                    trellis.setLED(cmd);

                    trellis.writeDisplay();
                    break;
                }
            }
            if (cmd != 0xff) {
#ifdef DEBUG
                Serial.print(F("* Sending -> \""));
                Serial.print((char)(cmd + 0x30));
                Serial.println("\"");
#endif
                // write the data
                BTLEserial.write(&cmd, 1);
            }
        }
    }
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
