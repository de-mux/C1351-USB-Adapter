/*
 * Design
 *
 * - IRQ every 512 uS: read pot_x & pot_y from C1351
 * - IRQ (slower):
 *   - read most recent pot_x and pot_y values
 *   - calculate difference
 *   - apply median filter
 *   - apply smoothing filter
 *   - send mouse movement cmd over USB
 *   - send mouse clicks over USB (combine w/ above if possible)
 *
 * Timers used:
 *  Timer 4 (main 256 uS interrupt)
 */

#include "controller.hpp"
#include "mouse.h"
#include "pins.hpp"


#define MAIN_INTERRUPT_INTERVAL_US 256


/* Mode for the main interrupt. Toggles between discharging and reading the
 * POTX/POTY pins.
 */
enum {
    POT_MODE_DISCHARGE = 0,
    POT_MODE_READ
};


void setDebugLow()
{
    PORT(PORT_DEBUG) &= ~_BV(PIN_DEBUG);
}


void setDebugHigh()
{
    PORT(PORT_DEBUG) |= _BV(PIN_DEBUG);
}


ISR(TIMER4_COMPA_vect)
{
    static volatile uint8_t mode = POT_MODE_DISCHARGE;

    if (mode == POT_MODE_DISCHARGE) {
        setDebugLow();
        c1351Sync();

        handleUsb();

        mode = POT_MODE_READ;
    }
    else {  // POT_MODE_READ
        setDebugHigh();
        c1351Read();

        mode = POT_MODE_DISCHARGE;
    }
    //PORTB |= _BV(PB6);
    //HID_Device_USBTask(&Mouse_HID_Interface);
    //USB_USBTask();
    //PORTB &= !_BV(PB6);
}


/* Setup main control interrupt (Timer 4 compare A) */
void setupMainInterrupt(int interval_us)
{
    cli();
    TCCR4A = 0;
    TCCR4B = 0;
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;
    TCNT4  = 0;
    OCR4A = (F_CPU / 1000000.0 / 16) * interval_us -
            1;             // compare match register duration (desired # cycles minus 1)
    //TCCR4B |= (1 << WGM42);   // CTC mode
    TCCR4B |= _BV(CS42) | _BV(CS40);    // 16x prescale
    TIMSK4 |= _BV(OCIE4A);  // enable timer compare interrupt
    sei();
}


void setupIO(void)
{
    DDR(PORT_DEBUG) |= _BV(PIN_DEBUG);
}


/* Main program entry point. */
int main(void)
{
    setupUsbMouse();
    setupIO();
    c1351Init();
    setupMainInterrupt(MAIN_INTERRUPT_INTERVAL_US);
    //setupControllerInterface();

    for (;;) {
        // For now, this happens in the interrupt, though we may want to move it here
        //handleUsb();
    }

    return 0;
}

