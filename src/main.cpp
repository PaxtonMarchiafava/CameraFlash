// using the compiler flag -Os


#include <Arduino.h>
#include <Wire.h> // i2c

#include <avr/sleep.h> // sleep
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/io.h>

// constants
#define brightnessLevels 10


// I2C address
#define APW7261 106
#define LM2759 83


// #define CameraFlash 0
#define FLASH 1
#define Button1 0
#define Button2 4

uint8_t torchBrightness = 0;
uint8_t FlashBrightness = 0;
volatile uint8_t flags = 0;

void sleepDevice() {
  sei();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}



void BrightnessDownButton () {
  PORTA.INTFLAGS = (1 << Button1);
  flags = 1;
}

void BrightnessUpButton () {
  PORTA.INTFLAGS = (1 << Button2);
  flags = 2;
}

void writeRegister(uint8_t device, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(device);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void setup() {

  pinMode(FLASH, OUTPUT);
  digitalWrite(FLASH, LOW);

  Wire.begin();
  writeRegister(LM2759, 0x10, 0x08); // shutdown

  // pinMode(2, INPUT); // read voltage at twi line to know battery voltage
  // pinMode(CameraFlash, INPUT_PULLUP);
  pinMode(Button1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button1), BrightnessDownButton, FALLING);
  pinMode(Button2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button2), BrightnessUpButton, FALLING);


  // Disable everything
  ADC0.CTRLA &= ~ADC_ENABLE_bm; // disable adc
  SPI0.CTRLA &= ~SPI_ENABLE_bm; // disable spi
  USART0.CTRLB &= ~USART_RXEN_bm; // disable usart rx?
  USART0.CTRLB &= ~USART_TXEN_bm; // disable usart tx?
  // TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // disable 16-bit Timer/Counter Type A
  // TCB0.CTRLA &= ~TCB_ENABLE_bm; // disable 16-bit Timer/Counter Type B
  AC0.CTRLA &= ~AC_ENABLE_bm; // disable Analog Comparator
  
  // TWI0.MCTRLA &= ~TWI_ENABLE_bm; // disable twi
  

  writeRegister(LM2759, 0xA0, torchBrightness); // set torch current

  writeRegister(LM2759, 0xB0, 0x09); // set Flash current
  writeRegister(LM2759, 0xC0, 0x02); // set Flash duration
  
  sei();

}

void loop() {

  if (flags & 1) { // brightness up
    flags = 0;
    
    if (torchBrightness < brightnessLevels) { // regular brightness up
      torchBrightness++;
      if (torchBrightness == 1) {
        writeRegister(LM2759, 0x10, 0x09); // torch mode
      }
      writeRegister(LM2759, 0xA0, torchBrightness - 1);
      while (!digitalRead(Button1)) {
        delay(1);
      }
    } else { // if brightness max, flash
      writeRegister(LM2759, 0x10, 0x08 + 0x03); // Set to flash mode
      delay(100);
      writeRegister(LM2759, 0x10, 0x08);
      writeRegister(LM2759, 0x10, 0x08 + 0x01); // set to torch mode
      writeRegister(LM2759, 0xA0, torchBrightness - 1);
    }
  }
  
  if ((flags & 2)) { // brightness down
    flags = 0;

    if (torchBrightness > 0) {
      torchBrightness--;
      if (torchBrightness <= 0) {
        writeRegister(LM2759, 0x10, 0x08); // set to shutoff
      } else {
        writeRegister(LM2759, 0xA0, torchBrightness - 1);
      }
      while (!digitalRead(Button2)) {
        delay(1);
      }
    }
  }

  delay(1);



}
