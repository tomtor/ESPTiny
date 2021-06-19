EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "ESP01 Attiny board"
Date "2021-06-19"
Rev "0.1"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ESP8266:ESP-01v090 U1
U 1 1 60CDAFCF
P 5200 3700
F 0 "U1" H 5200 4215 50  0000 C CNN
F 1 "ESP-01v090" H 5200 4124 50  0000 C CNN
F 2 "ESP8266:ESP-01" H 5200 3700 50  0001 C CNN
F 3 "http://l0l.org.uk/2014/12/esp8266-modules-hardware-guide-gotta-catch-em-all/" H 5200 3700 50  0001 C CNN
	1    5200 3700
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR01
U 1 1 60CDBEBC
P 3800 5200
F 0 "#PWR01" H 3800 5050 50  0001 C CNN
F 1 "+3V3" H 3815 5373 50  0000 C CNN
F 2 "" H 3800 5200 50  0001 C CNN
F 3 "" H 3800 5200 50  0001 C CNN
	1    3800 5200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 60CDC58A
P 6250 4800
F 0 "#PWR02" H 6250 4550 50  0001 C CNN
F 1 "GND" H 6255 4627 50  0000 C CNN
F 2 "" H 6250 4800 50  0001 C CNN
F 3 "" H 6250 4800 50  0001 C CNN
	1    6250 4800
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x10_Male J1
U 1 1 60CDCA2E
P 5150 5250
F 0 "J1" V 4985 5178 50  0000 C CNN
F 1 "Conn_01x10_Male" V 5076 5178 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x10_P2.54mm_Vertical" H 5150 5250 50  0001 C CNN
F 3 "~" H 5150 5250 50  0001 C CNN
	1    5150 5250
	0    1    1    0   
$EndComp
Wire Wire Line
	4650 4600 4250 4600
Wire Wire Line
	4250 4600 4250 3850
Wire Wire Line
	5550 5250 5550 4800
Wire Wire Line
	5550 4800 6250 4800
Wire Wire Line
	6150 3550 6250 3550
Wire Wire Line
	6250 3550 6250 4800
Connection ~ 6250 4800
Wire Wire Line
	3800 5200 4650 5200
Wire Wire Line
	4650 4600 4650 5200
Connection ~ 4650 5200
Wire Wire Line
	4650 5200 4650 5250
$Comp
L MCU_Microchip_ATtiny:ATtiny84-20PU U2
U 1 1 60CE8824
P 2850 2800
F 0 "U2" H 2321 2846 50  0000 R CNN
F 1 "ATtiny84-20PU" H 2321 2755 50  0000 R CNN
F 2 "Package_DIP:DIP-14_W7.62mm" H 2850 2800 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/doc8006.pdf" H 2850 2800 50  0001 C CNN
	1    2850 2800
	1    0    0    -1  
$EndComp
$Comp
L Diode:BAT85 D2
U 1 1 60CE9D54
P 4200 1700
F 0 "D2" H 4200 1917 50  0000 C CNN
F 1 "BAT85" H 4200 1826 50  0000 C CNN
F 2 "Diode_THT:D_DO-35_SOD27_P2.54mm_Vertical_AnodeUp" H 4200 1525 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/BAT85.pdf" H 4200 1700 50  0001 C CNN
	1    4200 1700
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 60CEAC44
P 2400 1250
F 0 "C3" H 2515 1296 50  0000 L CNN
F 1 "100n" H 2515 1205 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 2438 1100 50  0001 C CNN
F 3 "~" H 2400 1250 50  0001 C CNN
	1    2400 1250
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C2
U 1 1 60CEB38F
P 1800 1250
F 0 "C2" H 1918 1296 50  0000 L CNN
F 1 "100u" H 1918 1205 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_Tantal_D5.5mm_P5.00mm" H 1838 1100 50  0001 C CNN
F 3 "~" H 1800 1250 50  0001 C CNN
	1    1800 1250
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D1
U 1 1 60CEC3FD
P 1250 1550
F 0 "D1" H 1243 1767 50  0000 C CNN
F 1 "LED" H 1243 1676 50  0000 C CNN
F 2 "LED_THT:LED_D3.0mm" H 1250 1550 50  0001 C CNN
F 3 "~" H 1250 1550 50  0001 C CNN
	1    1250 1550
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 60CEC878
P 1150 2050
F 0 "R1" H 1220 2096 50  0000 L CNN
F 1 "220" H 1220 2005 50  0000 L CNN
F 2 "" V 1080 2050 50  0001 C CNN
F 3 "~" H 1150 2050 50  0001 C CNN
	1    1150 2050
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x10_Male J2
U 1 1 60CECE1E
P 5200 2400
F 0 "J2" V 5035 2328 50  0000 C CNN
F 1 "Conn_01x10_Male" V 5126 2328 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x10_P2.54mm_Vertical" H 5200 2400 50  0001 C CNN
F 3 "~" H 5200 2400 50  0001 C CNN
	1    5200 2400
	0    1    1    0   
$EndComp
$Comp
L Regulator_Linear:MCP1700-3302E_TO92 U3
U 1 1 60CEDF15
P 3450 1100
F 0 "U3" H 3450 951 50  0000 C CNN
F 1 "MCP1700-3302E_TO92" H 3450 860 50  0000 C CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 3450 900 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/20001826D.pdf" H 3450 1100 50  0001 C CNN
	1    3450 1100
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 60CEE919
P 1250 900
F 0 "C1" H 1365 946 50  0000 L CNN
F 1 "1u" H 1365 855 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D6.0mm_W2.5mm_P5.00mm" H 1288 750 50  0001 C CNN
F 3 "~" H 1250 900 50  0001 C CNN
	1    1250 900 
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04_Male J3
U 1 1 60CEFE49
P 1200 3200
F 0 "J3" H 1308 3481 50  0000 C CNN
F 1 "Conn_01x04_Male" H 1308 3390 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Horizontal" H 1200 3200 50  0001 C CNN
F 3 "~" H 1200 3200 50  0001 C CNN
	1    1200 3200
	1    0    0    -1  
$EndComp
$EndSCHEMATC
