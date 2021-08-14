build:
	#arduino-cli compile -b ATTinyCore:avr:attinyx4 --build-property build.extra_flags=-DF_CPU=8000000L -e ESPTiny44/ESPTiny44.ino
	arduino-cli compile -b megaTinyCore:megaavr:atxy4:chip=1614,clock=4internal,bodvoltage=1v8,bodmode=disabled,eesave=enable,millis=enabled,resetpin=UPDI,startuptime=0,uartvoltage=skip -e ESPTiny1614/ESPTiny1614.ino
