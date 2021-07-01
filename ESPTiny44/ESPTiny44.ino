#define led       7
#define reset_ESP 9

#include <TinyWire.h>

//#define BODS 7                   //BOD Sleep bit in MCUCR
//#define BODSE 2                  //BOD Sleep enable bit in MCUCR

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
// system wakes up when watchdog is timed out
void system_sleep() {
  //  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
  //  sbi(ACSR,ACD);                    //disable the analog comparator

  //uint8_t mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
  //uint8_t mcucr2 = mcucr1 & ~_BV(BODSE);
  //MCUCR = mcucr1;
  //MCUCR = mcucr2;

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out

  //sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
  //cbi(ACSR,ACD);
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
    clock_prescale_set(clock_div_32);
    delayMicroseconds(n * 1000 / 32);
    clock_prescale_set(clock_div_1);
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
#if 1 || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44)
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
  sbi(ACSR, ACD);                   //disable the analog comparator

  return results;
} // end of getBandgap


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


void blinkFastN(uint8_t n, uint8_t l = led)
{
  for (uint8_t i = 0; i < n; i++) {
    digitalWrite(l, HIGH);
    sleepDelay(2, false);
    digitalWrite(l, LOW);
    sleepDelay(64);
  }
}

// the setup routine runs once when you press reset:
void setup() {
  cbi(ADCSRA, ADEN);                // switch Analog to Digitalconverter OFF
  sbi(ACSR, ACD);                   //disable the analog comparator

  //  uint8_t mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
  //  uint8_t mcucr2 = mcucr1 & ~_BV(BODSE);
  //  MCUCR = mcucr1;
  //  MCUCR = mcucr2;

  pinMode(led, OUTPUT);

  for (byte i = 0; i < 3; i++) {
    blinkN(1, led);
  }

  sleepDelay(4096);
}


unsigned voltage;

volatile unsigned cnt;

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  // TinyWire.send("hi!!", 4); // respond with message of 4 bytes
  TinyWire.send(cnt >> 8);
  TinyWire.send(cnt & 0xFF);
  TinyWire.send(voltage >> 8);
  TinyWire.send(voltage & 0xFF);

  ++cnt;
}


// the loop routine runs over and over again forever:
void loop() {
  
  unsigned int v = voltage = getBandgap();
  blinkN(v / 1000);
  sleepDelay(1024);
  blinkN((v % 1000) / 100);

  for (int l = 0; l < 50; l++) {
    sleepDelay(8192);
    blinkN(1);
  }

  digitalWrite(reset_ESP, LOW);
  pinMode(reset_ESP, OUTPUT);
  sleepDelay(32);
  pinMode(reset_ESP, INPUT);

  delay(3000);
  
  auto startcnt = cnt;

  TinyWire.begin(1);                // join i2c bus with address #1
  TinyWire.onRequest(requestEvent); // register event
  
  auto end = millis() + 4000;
  while (cnt == startcnt && millis() < end)
    ;
  delay(200);
  TinyWire.end();
}
