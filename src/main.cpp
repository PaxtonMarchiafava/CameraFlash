
// using the compiler flag -Os

#include <Arduino.h>
#include <Wire.h> // i2c

#include <avr/sleep.h> // sleep
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/io.h>

// constants
#define maxBrightnessLevel 12 // max 16

#define brightnessUpFlag 1
#define brightnessDownFlag 2

// I2C address
#define LM2759 83

// #define button1 1 // pa7
// #define button2 4 // pa3

uint8_t torchBrightness = 0; // datasheet max 15, because wee add 0 as an option this var can be 16 as max
uint8_t FlashBrightness = 4; // datasheet max 15
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
uint8_t writeRegister(uint8_t device, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(device);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
  
  // if (Wire.endTransmission() > 0) {
  //   batteryDead = 1;
  // } else {
  //   batteryDead = 1;
  // }
}

void writeBrightness() {
  if (torchBrightness > 0) {
    writeRegister(LM2759, 0xA0, (torchBrightness - 1));
    writeRegister(LM2759, 0x10, 0x09); // torch mode
  } else {
    writeRegister(LM2759, 0x10, 0x08); // set to shutoff
  }
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
  
  // CCP = CCP_IOREG_gc;
  // CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSCULP32K_gc;
  
  // BOD.CTRLA &= BOD_SAMPFREQ_1KHZ_gc;
}

void gpioSetup () {

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);

  PORTA.PIN3CTRL = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc;
  PORTA.PIN7CTRL = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc;

  PORTA_DIR = PIN6_bm; // extra pin as out

  PORTA_OUTSET = PIN6_bm;
  delay(100);
  PORTA_OUTCLR = PIN6_bm;

  // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // sei();
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
  writeRegister(LM2759, 0xB0, FlashBrightness); // set Flash current
  writeRegister(LM2759, 0xC0, 0x02); // set Flash duration

}

void setup() {
  gpioSetup();
  i2cSetup();
  // disablePeripherals();

  delay(500);
  writeRegister(LM2759, 0xA0, 1);
  writeRegister(LM2759, 0x10, 0x09); // torch mode  

  // change clock speeds need to test
  // _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);   // 20 MHz
  // _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PDIV_4X_gc | CLKCTRL_PEN_bm);  // 5 MHz

  PORTA_OUTSET = PIN6_bm;
  delay(100);
  PORTA_OUTCLR = PIN6_bm;
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
  if (torchBrightness < maxBrightnessLevel) { // brightness below max
    torchBrightness++;

  } else if (torchBrightness >= maxBrightnessLevel) { // wrap back around from max brightness to off
    torchBrightness = 0;
  }
  writeBrightness();

  /* else if ((torchBrightness >= maxBrightnessLevel)) { // flash
    writeRegister(LM2759, 0xB0, FlashBrightness); // set Flash current
    writeRegister(LM2759, 0x10, 0x08 + 0x03); // Set to flash mode
    delay(10);
    writeRegister(LM2759, 0xA0, torchBrightness - 1);
    writeRegister(LM2759, 0x10, 0x08 + 0x01); // set to torch mode
  
  }*/

  while (!digitalRead(button1)) {}
}

// Decreases the torch mode brightness
void brightnessDown () {
  if (torchBrightness > 0) {
    torchBrightness--;

  } else if (torchBrightness <= 0) { // torch already at 0, wrap around to max brightness
    torchBrightness = maxBrightnessLevel;
  }
  writeBrightness();


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

  // torchBrightness = 2;
  // writeBrightness();
  // delay(10);  
}
