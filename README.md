#laptop-remote

##What is it?
This is a simple project that combines a [Trellis button
pad](https://www.adafruit.com/products/1616) and a [nRF8001 BlueTooth
Low Energy](https://www.adafruit.com/products/1697) breakout in order
to run commands remotely on my (linux) laptop. This project uses the
Arduino Uno for the remote, and a shell script as the receiver.

##Motivation
I thought it would be cool to control my laptop when watching a movie
or other media. It also served as a learning experience for the BLE
protocol.

##Dependencies
Arduino Libraries:

* [Adafruit Trellis](https://github.com/adafruit/Adafruit_Trellis_Library)
* [Adafruit BLE UART](https://github.com/adafruit/Adafruit_nRF8001)

Linux commands:

* gatttool

##Status
- [x] Working prototype on breadboard
- [x] Transfer off of Arduino board
- [x] Design PCB
- [ ] Design case
- [ ] Etch PCB
- [ ] Build case
- [ ] Done!

##Future Goals
Because it uses BlueTooth, it doesn't matter /what/ exactly is connecting
to my laptop. That being said, I might write an Android application to
do the same.
