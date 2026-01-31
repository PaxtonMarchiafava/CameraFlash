// todo: Only one pin interrupt works while in sleep
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

#define brightnessUpFlag 1
#define brightnessDownFlag 2

// I2C address
#define APW7261 106
#define LM2759 83

// #define CameraFlash 0
#define FLASH 1
#define button1 0 // pa6
#define button2 4 // pa3

uint8_t torchBrightness = 0;
uint8_t FlashBrightness = 0;
volatile uint8_t flags = 0;

/*
┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐
│  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  │
│  │└──────┘└──────┘└──────┘└──────┘└──────┘│  │
│  │          Everything Functions          │  │
│  │┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐│  │
│  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  │  
└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘
*/

// Write i2c data
void writeRegister(uint8_t device, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(device);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

/*
┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐
│  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  │
│  │└──────┘└──────┘└──────┘└──────┘└──────┘│  │
│  │             Setup Functions            │  │
│  │┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐│  │
│  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  │  
└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘
*/

// button1 interrupt routine
void ButtonInterrupt () {
  // PORTA.INTFLAGS = (1 << Button1);
  if (digitalRead(button1)) {
    flags |= 1;
  }

  if (digitalRead(button2)) {
    flags |= 2;
  }
  
}

void disablePeripherals() {
  ADC0.CTRLA &= ~ADC_ENABLE_bm; // disable adc
  SPI0.CTRLA &= ~SPI_ENABLE_bm; // disable spi
  USART0.CTRLB &= ~USART_RXEN_bm; // disable usart rx?
  USART0.CTRLB &= ~USART_TXEN_bm; // disable usart tx?
  TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // disable 16-bit Timer/Counter Type A
  TCB0.CTRLA &= ~TCB_ENABLE_bm; // disable 16-bit Timer/Counter Type B
  AC0.CTRLA &= ~AC_ENABLE_bm; // disable Analog Comparator
  // TWI0.MCTRLA &= ~TWI_ENABLE_bm; // disable twi
}

void gpioSetup () {

/* only works in SLEEP_MODE_IDLE
pinMode(button1, INPUT_PULLUP);
pinMode(button2, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(button1), buttonPressISR, FALLING);
attachInterrupt(digitalPinToInterrupt(button2), buttonPressISR, FALLING);
*/

  pinMode(FLASH, OUTPUT);
  digitalWrite(FLASH, LOW);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);

  PORTA.PIN3CTRL = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc;
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc;
  // PORTA.INTFLAGS Gets set on interrupt
  // NEED to clear PORTA.INTFLAGS in isr

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sei();
}

ISR(PORTA_PORT_vect) {
  PORTA.INTFLAGS = 0xFF; // reset interrupt flag
  if (!digitalRead(button1)) {
    flags = brightnessUpFlag;
  }else if (!digitalRead(button2)) {
    flags = brightnessDownFlag;
  }
}

void i2cSetup () {
  Wire.begin();
  
  writeRegister(LM2759, 0x10, 0x08); // shutdown led driver
  writeRegister(LM2759, 0xA0, 0); // set torch current
  writeRegister(LM2759, 0xB0, 0x06); // set Flash current
  writeRegister(LM2759, 0xC0, 0x02); // set Flash duration

}

void setup() {
  gpioSetup();
  i2cSetup();
  disablePeripherals();
}

/*
┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐
│  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  │
│  │└──────┘└──────┘└──────┘└──────┘└──────┘│  │
│  │             Loop Functions             │  │
│  │┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐│  │
│  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  │  
└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘
*/

// set sleep modes
void sleepDevice() {
  // SLEEP_MODE_IDLE, SLEEP_MODE_STANDBY, SLEEP_MODE_PWR_DOWN
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  // sleep_enable();
  sei(); // enable interrupt
  sleep_mode();
  cli(); // disable interrupt

  // sleep_disable();
}

// Increases the torch mode brightness
void brightnessUp () {
  if (torchBrightness < brightnessLevels) {
    torchBrightness++;
    if (torchBrightness == 1) {
      writeRegister(LM2759, 0x10, 0x09); // torch mode
    }
    writeRegister(LM2759, 0xA0, torchBrightness - 1);
  }
  while (!digitalRead(button1)) {}
}

// Decreases the torch mode brightness
void brightnessDown () {
  if (torchBrightness > 0) {
    torchBrightness--;
    if (torchBrightness <= 0) {
      writeRegister(LM2759, 0x10, 0x08); // set to shutoff
    } else {
      writeRegister(LM2759, 0xA0, torchBrightness - 1);
    }
  }
  while (!digitalRead(button2)) {}
}

void loop() {

  if (flags == brightnessUpFlag) {
    flags = 0;
    brightnessUp();
  } else if (flags == brightnessDownFlag) {
    flags = 0;
    brightnessDown();
  }
  
  sleepDevice();
  
}


/*
void loop() {
  
if (!digitalRead(button1)) { // brightness up
// flags = 0;
brightnessUp();
//   writeRegister(LM2759, 0x10, 0x09); // torch mode
//   writeRegister(LM2759, 0xA0, 0);

// } else {
  //   writeRegister(LM2759, 0x10, 0x08); // off
  while (!digitalRead(button1)) {}
}

if (!digitalRead(button2)) { // brightness down
// flags = 0;
brightnessDown();
while (!digitalRead(button2)) {}
}

sleepDevice();
}
*/

/*
┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐
│  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  │
│  │└──────┘└──────┘└──────┘└──────┘└──────┘│  │
│  │               Old Stuff                │  │
│  │┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐│  │
│  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  ┌┐  └┘  │  
└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘

    // if at max brightness, flash
    } else { // if brightness max, flash
      writeRegister(LM2759, 0x10, 0x08 + 0x03); // Set to flash mode
      delay(100);
      writeRegister(LM2759, 0x10, 0x08);
      writeRegister(LM2759, 0x10, 0x08 + 0x01); // set to torch mode
      writeRegister(LM2759, 0xA0, torchBrightness - 1);
    }

*/




