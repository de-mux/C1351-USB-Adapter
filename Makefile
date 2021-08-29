BUILD_DIR=.pio/build/itsybitsy32u4_5V
DEBUG_BUILD_DIR=.pio/build/debug

all:
	pio run -v
	pio run -t compiledb

upload:
	pio run -t upload
	pio run -t compiledb

clean:
	pio run -t clean
	pio run -e debug -t clean

debug:
	pio run -e debug -v
	avr-objdump -S ${DEBUG_BUILD_DIR}/firmware.elf > ${DEBUG_BUILD_DIR}/firmware.s

compiledb:
	pio run -t compiledb
