#define led       PIN_PA7
#define reset_ESP PIN_PB3

#include <Wire.h>

#include <avr/sleep.h>
//#include <avr/wdt.h>
//#include <avr/power.h>

void RTC_init(void)
{
  /* Initialize RTC: */
  while (RTC.STATUS > 0)
  {
    ;                                   /* Wait for all register to be synchronized */
  }
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */

  RTC.PITINTCTRL = RTC_PI_bm;           /* PIT Interrupt: enabled */
}

ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
}


void sleepDelay(unsigned n, boolean off = true)
{
  unsigned ticks = 1;
  byte period = RTC_PERIOD_CYC16384_gc; // 512 ms
  switch (n) {
    //case 1: period = RTC_PERIOD_CYC32_gc; break;
    case 1: period = RTC_PERIOD_CYC64_gc; break;
    case 2: period = RTC_PERIOD_CYC128_gc; break;
    case 4: period = RTC_PERIOD_CYC256_gc; break;
    case 8: period = RTC_PERIOD_CYC512_gc; break;
    case 16: period = RTC_PERIOD_CYC1024_gc; break;
    case 32: period = RTC_PERIOD_CYC2048_gc; break;
    case 64: period = RTC_PERIOD_CYC4096_gc; break;
    case 128: period = RTC_PERIOD_CYC8192_gc; break;
    case 512: case 500: break;
    case 1024: case 1000: ticks = 2; break;
    case 2048: case 2000: ticks = 4; break;
    case 4096: case 4000: ticks = 8; break;
    case 8192: case 8000: ticks = 16; break;
    default: ticks = n+1; period = RTC_PERIOD_CYC32_gc; break; // 1 ms resolution
  }
  
  while (RTC.PITSTATUS & 1)
    ;
  
  RTC.PITCTRLA = period | RTC_PITEN_bm;    /* Enable PIT counter: enabled */
  
  while (RTC.PITSTATUS & 1)
    ;
  
  while (ticks) {
    ticks--;
    sleep_cpu();
  }
}

#if 1
unsigned int getBandgap ()
{
  VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc;    /* Set the Vref to 1.1V*/

  /* The following section is partially taken from Microchip App Note AN2447 page 13*/

  ADC0.MUXPOS = ADC_MUXPOS_INTREF_gc    /* ADC internal reference, the Vbg*/;
  ADC0.CTRLC = ADC_PRESC_DIV4_gc        /* CLK_PER divided by 4 */
               | ADC_REFSEL_VDDREF_gc   /* Vdd (Vcc) be ADC reference */
               | 0 << ADC_SAMPCAP_bp;   /* Sample Capacitance Selection: disabled */

  uint16_t Vcc_value = 0;                /* measured Vcc value */

  ADC0.CTRLA = 1 << ADC_ENABLE_bp     /* ADC Enable: enabled */
               | 1 << ADC_FREERUN_bp  /* ADC Free run mode: enabled */
               | ADC_RESSEL_10BIT_gc; /* 10-bit mode */

  ADC0.COMMAND |= 1;                  // start running ADC
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  Vcc_value = 0x400 * 1100L / ADC0.RES /* calculate the Vcc value */;
  
  return Vcc_value;
} // end of getBandgap
#endif

#if 0
unsigned int getBattery ()
{
  int val = analogRead(A1);

  // 3.0v regulator with voltage divider (820k/820k):
  const float Vref = 3000,
              R = 820000,
              Rintern = 100000000, // ATtiny ADC internalresistance
              Rpar = (R * Rintern) / (R + Rintern);
  const long scale = Vref * (R + Rpar) / Rpar;
  unsigned int results = scale * val / 1024;

  return results;
} // end of getBattery
#endif


void blinkN(uint8_t n, uint8_t l = led)
{
  for (uint8_t i = 0; i < n; i++) {
    digitalWrite(l, HIGH);
    sleepDelay(2, false);
    digitalWrite(l, LOW);
    sleepDelay(512);
    sleepDelay(128);
  }
}


// the setup routine runs once when you press reset:
void setup() {
  RTC_init();                           /* Initialize the RTC timer */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  /* Set sleep mode to POWER DOWN mode */
  sleep_enable();                       /* Enable sleep mode, but not going to sleep yet */

  pinMode(PIN_PA1, INPUT_PULLUP);
  pinMode(PIN_PA2, INPUT_PULLUP);
  pinMode(PIN_PA3, INPUT_PULLUP);
  pinMode(PIN_PA4, INPUT_PULLUP);
  pinMode(PIN_PA5, INPUT_PULLUP);
  pinMode(PIN_PA6, INPUT_PULLUP);
  pinMode(PIN_PA7, INPUT_PULLUP);
  pinMode(PIN_PB0, INPUT_PULLUP);
  pinMode(PIN_PB1, INPUT_PULLUP);
  pinMode(PIN_PB2, INPUT_PULLUP);
  pinMode(PIN_PB3, INPUT_PULLUP);

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
  
  //unsigned int v = voltage = getBattery();
  unsigned int v = voltage = getBandgap();
  blinkN(v / 1000);
  sleepDelay(1500);
  blinkN((v % 1000) / 100);
  sleepDelay(1500);
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

#if 0
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
