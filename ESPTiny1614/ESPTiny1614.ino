#define led       PIN_PA7

// #define reset_ESP PIN_PB3
#define reset_ESP PIN_PA5

#define USE_BME  0
#define USE_ESP 0

#if USE_ESP || USE_BME
#include <Wire.h>
#endif

#include <avr/sleep.h>


#if USE_BME
#include <BME280I2C.h>

BME280I2C bme;
#endif

void RTC_init(void)
{
  /* Initialize RTC: */
  while (RTC.STATUS > 0)
  {
    ;                                   /* Wait for all register to be synchronized */
  }
#if 1
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */
#else  
  RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;   /* 32.768kHz External Oscillator */
#endif

  RTC.PITINTCTRL = RTC_PI_bm;           /* PIT Interrupt: enabled */

  // For 2s overflow ticks while sleeping:
  RTC.INTCTRL = RTC_OVF_bm;
  RTC.CTRLA = RTC_PRESCALER_DIV1_gc | RTC_RUNSTDBY_bm | RTC_RTCEN_bm;
}


volatile uint16_t ticks;

ISR(RTC_PIT_vect)
{
  if (ticks)
    ticks--;
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
}


volatile uint8_t sec2s, hours, minutes; // 2s ticks

ISR(RTC_CNT_vect)
{
  if (sec2s < 58)
    sec2s += 2;
  else {
    sec2s = 0;
    if (minutes < 59)
      minutes++;
    else {
      minutes = 0;
      if (hours < 23)
        hours++;
      else
        hours = 0;
    }  
  }

  RTC.INTFLAGS = RTC_OVF_bm;          /* Clear interrupt flag by writing '1' (required) */
}


void sleepDelay(uint16_t n)
{
  uint8_t period;

#if 0
  if (n < 10) {
    period = RTC_PERIOD_CYC4_gc; // 1/8 ms
    ticks = (n << 3);
  } else
#endif
  if (n < 100) {
      period = RTC_PERIOD_CYC8_gc; // 1/4 ms
      ticks = (n << 2);
  } else if (n < 1000) {
      period = RTC_PERIOD_CYC64_gc; // 2 ms
      ticks = (n >> 1);
  } else if (n < 5000) {
      period = RTC_PERIOD_CYC1024_gc; // 32 ms
      ticks = (n >> 5);
  } else {
      period = RTC_PERIOD_CYC8192_gc; // 256 ms
      ticks = (n >> 8);
  }

  while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)  // Wait for new settings to synchronize
    ;

  RTC.PITCTRLA = period | RTC_PITEN_bm;    // Enable PIT counter: enabled

  while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)  // Wait for new settings to synchronize
    ;

  while (ticks)
    sleep_cpu();

  RTC.PITCTRLA = 0;    /* Disable PIT counter */
}

#if 1
unsigned int getBandgap ()
{
#if 0
  analogReference(INTERNAL1V1);
  uint16_t Vcc_value = 0x400 * 1100UL / analogRead(ADC_INTREF); /* calculate the Vcc value */
#else
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

  Vcc_value = 0x400 * 1100UL / ADC0.RES /* calculate the Vcc value */;
#endif
  return Vcc_value;
} // end of getBandgap
#endif

#if 1
unsigned int getBattery ()
{
  unsigned int val = analogRead(PIN_PA3);

  // 3.0v regulator with voltage divider (820k/820k):
  const float Vref = 3000,
              R = 820000,
              Rintern = 100000000, // ATtiny ADC internalresistance
              Rpar = (R * Rintern) / (R + Rintern);
  const unsigned long scale = Vref * (R + Rpar) / Rpar;
  unsigned int results = scale * val / 0x400;

  return results;
} // end of getBattery
#endif


void blinkN(uint8_t n, uint8_t l = led)
{
  for (uint8_t i = 0; i < n; i++) {
    digitalWrite(l, HIGH);
    sleepDelay(2);
    digitalWrite(l, LOW);
    sleepDelay(700);
  }
}


void blinkDec(uint8_t d)
{
  blinkN(d / 10);
  sleepDelay(1500);
  blinkN(d % 10);
  sleepDelay(3000);
}


// the setup routine runs once when you press reset:
void setup() {
  RTC_init();                           /* Initialize the RTC timer */
  //set_sleep_mode(SLEEP_MODE_PWR_DOWN);  /* Set sleep mode to POWER DOWN mode */
  set_sleep_mode(SLEEP_MODE_STANDBY);
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

  sleepDelay(5000);
}


unsigned voltage;

volatile unsigned cnt;

#if USE_ESP
// function that executes whenever data is requested by master
// this function is registered as an event
void requestEvent() {
  Wire.write(cnt >> 8);
  Wire.write(cnt & 0xFF);
  Wire.write(voltage >> 8);
  Wire.write(voltage & 0xFF);

  ++cnt;
}
#endif


// the loop routine runs over and over again forever:
void loop() {

  //unsigned int v = voltage = getBattery();
  unsigned int v = voltage = getBandgap();
  blinkDec((v + 50) / 100);

  blinkDec(minutes);
  blinkDec(sec2s);

  for (byte l = 0; l < 60000 / (5000 + 2 + 700); l++) {  // 60s / delay per loop (5000 + blink time)
    blinkN(1);
    sleepDelay(5000);
  }

#if USE_BME
  Wire.begin(1);                // join i2c bus with address #1
  bme.begin();
#if 0
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);
#else
  static float oldtemp;
  float temp;
  if ((temp = bme.temp()) == oldtemp)
    return;
  oldtemp = temp;
#endif
#endif

#if USE_ESP
  digitalWrite(reset_ESP, LOW);
  pinMode(reset_ESP, OUTPUT);
  sleepDelay(32);
  pinMode(reset_ESP, INPUT_PULLUP);

  sleepDelay(3000);

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
