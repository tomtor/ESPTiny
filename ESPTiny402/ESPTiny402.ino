// #include <Event.h>
// #include <Logic.h>

#include <avr/sleep.h>

#define led       PIN_PA7
#define reset_ESP PIN_PB3

#define USE_BME	0
#define USE_ESP 0

#if USE_ESP
#include <Wire.h>
#endif

#if USE_BME
#include <BME280I2C.h>

BME280I2C bme;
#endif

#define SLEEPINT

void RTC_init(void)
{
  /* Initialize RTC: */
  while (RTC.STATUS > 0)
    ;                                   /* Wait for all register to be synchronized */
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */

  RTC.CTRLA |= RTC_RTCEN_bm | RTC_RUNSTDBY_bm;
}

#ifdef SLEEPINT
volatile uint8_t sleep_cnt;

ISR(RTC_CNT_vect)
{
  RTC.INTFLAGS = RTC_CMP_bm;            /* Clear interrupt flag by writing '1' (required) */
  sleep_cnt--;
}
#else
ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
}
#endif


void sleepDelay(uint16_t n)
{
#ifndef SLEEPINT
  uint16_t ticks;
  uint8_t period;

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
 
  RTC.PITINTCTRL |= RTC_PI_bm;             /* PIT Interrupt: enabled */

  while (ticks--)
    sleep_cpu();

  //RTC.PITCTRLA = 0;    /* Disable PIT counter, should not be done, see erratum */
  RTC.PITINTCTRL &= ~RTC_PI_bm;           /* PIT Interrupt: disabled */
#else
  while (RTC.STATUS /* & RTC_CMPBUSY_bm */)  // Wait for new settings to synchronize
    ;

  uint32_t delay;
  RTC.CMP = RTC.CNT + (delay = (n * 32UL) + uint16_t(n / 128 * 3)); // With this calculation every multiple of 128ms is exact!

  while (RTC.STATUS /* & RTC_CMPBUSY_bm */)  // Wait for new settings to synchronize
    ;
  
  sleep_cnt = delay / 65536 + 1; // Calculate number of wrap arounds (overflows)
  uint64_t start = millis();
  RTC.INTCTRL |= RTC_CMP_bm;
  while (sleep_cnt) {
    sleep_cpu();
    if (sleep_cnt)
      nudge_millis(2000);
  }
  set_millis(start + n);

  RTC.INTCTRL &= ~RTC_CMP_bm;
#endif
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
    digitalWriteFast(l, HIGH);
    sleepDelay(2);
    digitalWriteFast(l, LOW);
    sleepDelay(700);
  }
}


// the setup routine runs once when you press reset:
void setup() {
  // Serial.swap(1); // A1,A2 = TX,RX
  // Serial.begin(9600);
  // Serial.println("Start");
  // while (1)
  //   Serial.println(TCA0.SPLIT.HCNT);

  RTC_init();                           /* Initialize the RTC timer */
  #ifdef SLEEPINT
  set_sleep_mode(SLEEP_MODE_STANDBY);
  #else
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  #endif
  sleep_enable();                       /* Enable sleep mode, but not going to sleep yet */

  pinMode(PIN_PA1, INPUT_PULLUP);
  pinMode(PIN_PA2, INPUT_PULLUP);
  pinMode(PIN_PA3, INPUT_PULLUP);
  //  pinMode(PIN_PA4, INPUT_PULLUP);
  //  pinMode(PIN_PA5, INPUT_PULLUP);
  pinMode(PIN_PA6, INPUT_PULLUP);
  pinMode(PIN_PA7, INPUT_PULLUP);
  //  pinMode(PIN_PB0, INPUT_PULLUP);
  //  pinMode(PIN_PB1, INPUT_PULLUP);
  //  pinMode(PIN_PB2, INPUT_PULLUP);
  //  pinMode(PIN_PB3, INPUT_PULLUP);

  pinMode(led, OUTPUT);

  for (byte i = 0; i < 3; i++) {
    blinkN(1, led);
  }

  sleepDelay(5000);

  // pinMode(PIN_PA2, OUTPUT);
  // Event0.set_generator(gen::rtc_ovf);
  // Event0.set_user(user::evouta_pin_pa2);
  // Event0.start();
  // pinMode(PIN_PA7, OUTPUT);
  // Logic1.enable = true;
  // Logic1.input0 = logic::in::tca0;
  // Logic1.input1 = logic::in::tca0;
  // Logic1.input2 = logic::in::tca0;
  // Logic1.output = logic::out::enable;
  // Logic1.filter = logic::filter::disable;
  // Logic1.truth = 0x55;
  // Logic1.init();
  // Logic1.start();
}


unsigned voltage;

#if USE_ESP
volatile unsigned cnt;

// function that executes whenever data is requested by master
// this function is registered as an event
void requestEvent() {
  Wire.write(cnt >> 8);
  Wire.write(cnt & 0xF0);
  Wire.write(voltage >> 8);
  Wire.write(voltage & 0xFF);

  ++cnt;
}
#endif

// the loop routine runs over and over again forever:
void loop() {

  //unsigned int v = voltage = getBattery();

  unsigned int v = voltage = getBandgap();
  blinkN(v / 1000);
  sleepDelay(1500);
  blinkN((v % 1000) / 100);
  sleepDelay(1500);
  blinkN((v % 100) / 10);

  // sleepDelay(5000);
  // while (1)
  //   digitalWriteFast(led, RTC.CNT & 1 ? HIGH: LOW);

  sleepDelay(5000);
  for (int i= 1; i <= 10; i++) {
    sleepDelay(1);
    digitalWriteFast(led, HIGH);
    sleepDelay(i);
    digitalWriteFast(led, LOW);
  }
  sleepDelay(5000);

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
