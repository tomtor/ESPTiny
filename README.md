# ATtiny84/16x4/x02 and ESP-01 (ESP8266) KiCad PCB and example code

## Usage

The main function of the ATtiny is acting as a battery friendly low power co-processor for the ESP.
A bit like the ULP low power co-processor of the ESP32, except with a lot more IO functions and program memory.

ATtiny and ESP combined use 12uA in sleep mode or even less when the ESP
is switched off by the ATtiny with CS.

![3D rendering](ESPTiny.png)
![3D rendering](ESPTinyEmpty.png)

![3D rendering](ESPTiny1614.png)
![3D rendering](ESPTiny1614Empty.png)

![3D rendering](ESPTiny402.png)
![3D rendering](ESPTiny402Empty.png)

Usable on breadboard (ATtiny84: 9 rows high, leaves 2 rows empty and ATtiny16x4: 8 rows high) OR stand alone.

3 PCB designs: ATtiny84 or ATtiny16x4 or ATtinyx02

All components / features are optional:

- ATtiny84 or ATtiny1604 / ATtiny1614 or ATtiny202 / ATtiny402
- ESP-01 (When not used the connections/space can be used for any other (I2C) device)
- LED with resistor
- MCP1700 low quiescent current regulator
- BAT85 diode for Solar Charging panel (see https://www.thethingsnetwork.org/labs/story/a-cheap-stm32-arduino-node/step/solar-power)
- Voltage divider for measuring battery voltage
- Decoupling capacitors
- I2C communication between ESP and ATtiny (with optional 4k7 pull-up)
- ATtiny can restart ESP by pulling ESP Reset low

## Example code

The ATtiny wakes up the ESP every 10 minutes by pulling RST low and sends a counter and the supply voltage to it.

### OTA for the ESP
OTA for ESP program updates is activated for 20s when the I2C communication fails.
This can be triggered by resetting the ESP by pulling the RST line low.
Because the ATtiny is most of the time sleeping
the I2C communication will fail.

There is a small chance that the ATtiny will reset the ESP during upload when the 10 minute interval is completed.
You can prevent this by pulling the ATtiny RST low when using OTA, or just upload again.

## Schemas

https://github.com/tomtor/ESPTiny/raw/master/ESPTiny.pdf
<img alt="Schema" src="./ESPTiny.svg">

https://github.com/tomtor/ESPTiny/raw/master/ESPTiny1614.pdf
<img alt="Schema" src="./ESPTiny1614.svg">

https://github.com/tomtor/ESPTiny/raw/master/ESPTiny402.pdf
<img alt="Schema" src="./ESPTiny402.svg">

## Credits

https://github.com/jdunmire/kicad-ESP8266
