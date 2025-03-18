rm -rf build
rm *.lst *.map
arduino-cli compile -e --fqbn megaTinyCore:megaavr:atxy2
