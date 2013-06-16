# Blinky: Hello World for the Adafruit Atmega32u4 Breakout Board

This is a very simple program to verify that all of your AVR related stars
(tools) have aligned and can successfully program an AVR chip. The program
blinks an LED by toggling an IO port on and off. This specific program was
written to toggle the on-board (green) LED on an [Adafruit Atmega32u4 Breakout
Board][board].

## Requirements

* Make
* avr-gcc
* avr-objcopy
* avr-libc

## Building/Running

1. Connect your micro-controller to your computer/programmer
2. Make any necessary changes to the Makefile (for example, updating COM to the
   appropriate location of your device)
3. Make sure the device is programmable. For example, the Adafruit Atmega32u4
   uses a bootloader which is only active for a few seconds after a reset.
   If you're using this chip, the LED will pulse (not blink) on and off to
   indicate the bootloader is active.
4. Execute `make program`
5. Blinky

[board]: http://www.adafruit.com/products/296
