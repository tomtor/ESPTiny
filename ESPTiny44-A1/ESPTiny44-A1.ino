#define led       7
#define reset_ESP 9

#include <Wire.h>

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// set system into the sleep state
// system wakes up when watchdog times out
void system_sleep() {
  GIMSK &= ~(_BV(PCIE0) | _BV(PCIE1));   // No Pin Change Interrupts

  USICR &= ~(_BV(USISIE) | _BV(USIOIE)); // No USI interrupts

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // sleep mode is set here
  sleep_enable();

  sleep_mode();                         // System sleeps here

  sleep_disable();                      // System continues execution here when watchdog timed out
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(uint8_t ii) {

  byte bb;
  if (ii > 9) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  WDTCSR |= B01000000; // prevent reset if wakeup is too long
}


void sleepDelay(unsigned n, boolean off = true)
{
  if (n < 16) {
#if F_CPU==4000000L
    clock_prescale_set(clock_div_32); // was div_2
    delayMicroseconds(n * 1000 / 16);
    clock_prescale_set(clock_div_2);
#else
    clock_prescale_set(clock_div_32);
    delayMicroseconds(n * 1000 / 32);
    clock_prescale_set(clock_div_1);
#endif
  } else {
    byte v = 0;
    while (n >>= 1)
      v++;
    v -= 4;
    if (off) {
      pinMode(led, INPUT); // set all used ports to input to save power
    }
    setup_watchdog(v);
    system_sleep();
    pinMode(led, OUTPUT);
  }
}


// https://github.com/cano64/ArduinoSystemStatus/blob/master/SystemStatus.cpp
unsigned int getBandgap ()
{
  sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON
  cbi(ACSR, ACD);

  //reads internal 1V1 reference against VCC
#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44__)
  ADMUX = _BV(MUX5) | _BV(MUX0); // For ATtiny84
#elif defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__)
  ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85
#elif defined(__AVR_ATmega1284P__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega1284
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
#endif
  delay(2); // Wait for Vref to settle
  unsigned int val;
  for (byte i= 0; i < 2; i++) {
    ADCSRA |= _BV(ADSC); // Convert
    while (bit_is_set(ADCSRA, ADSC));
    uint8_t low = ADCL;
    val = (ADCH << 8) | low;
  }

  unsigned int results = ((long)1024 * 1085) / val;  // 1100 default, 1085 is calibrated value

  cbi(ADCSRA, ADEN);                // switch Analog to Digitalconverter OFF
  sbi(ACSR, ACD);                   // disable the analog comparator

  return results;
} // end of getBandgap


unsigned int getBattery ()
{
  sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON
  cbi(ACSR, ACD);
  
  int val = analogRead(A1);

  // 3.0v regulator with voltage divider (820k/820k):
  const float Vref = 3000,
              R = 820000,
              Rintern = 100000000, // ATtiny ADC internalresistance
              Rpar = (R * Rintern) / (R + Rintern);
  const long scale = Vref * (R + Rpar) / Rpar;
  unsigned int results = scale * val / 1024;

  cbi(ADCSRA, ADEN);                // switch Analog to Digitalconverter OFF
  sbi(ACSR, ACD);                   // disable the analog comparator

  return results;
} // end of getBattery


void blinkN(uint8_t n, uint8_t l = led)
{
  for (uint8_t i = 0; i < n; i++) {
    digitalWrite(l, HIGH);
    sleepDelay(2, false);
    digitalWrite(l, LOW);
    sleepDelay(512);
    sleepDelay(256);
  }
}


// the setup routine runs once when you press reset:
void setup() {
  cbi(ADCSRA, ADEN);                // switch Analog to Digitalconverter OFF
  sbi(ACSR, ACD);                   // disable the analog comparator

  pinMode(led, OUTPUT);

  for (byte i = 0; i < 3; i++) {
    blinkN(1, led);
  }

  sleepDelay(4096);
}


unsigned voltage;

volatile unsigned cnt;

// function that executes whenever data is requested by master
// this function is registered as an event
void requestEvent() {
  Wire.write(cnt >> 8);
  Wire.write(cnt & 0xFF);
  Wire.write(voltage >> 8);
  Wire.write(voltage & 0xFF);

  ++cnt;
}


// the loop routine runs over and over again forever:
void loop() {
  
  unsigned int v = voltage = getBattery(); // getBandgap();
  blinkN(v / 1000);
  sleepDelay(1024);
  blinkN((v % 1000) / 100);
  sleepDelay(1024);
  blinkN((v % 100) / 10);
  
  for (byte l = 0; l < 600000 / (8192 + 2 + 256 + 512); l++) {  // 600s / delay per loop (8192 + blink time)
#if 0
    if (v > 3750) { // High voltage, burn more energy
      digitalWrite(led, HIGH);
      delay(8192);
    } else
#endif
      sleepDelay(8192);
    blinkN(1);
  }

#if 1
  digitalWrite(reset_ESP, LOW);
  pinMode(reset_ESP, OUTPUT);
  sleepDelay(32);
  pinMode(reset_ESP, INPUT);

  sleepDelay(1024);
  sleepDelay(1024);
  sleepDelay(1024);
  
  auto startcnt = cnt;

  Wire.begin(1);                // join i2c bus with address #1
  Wire.onRequest(requestEvent); // register event
  
  auto end = millis() + 4000;
  while (cnt == startcnt && millis() < end)
    ;
  delay(200);
  Wire.end();
#endif
}
