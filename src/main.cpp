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


#define MAIN_INTERRUPT_INTERVAL_US 256


C1351Interface c1351;

/* Mode for the main interrupt. Toggles between discharging and reading the
 * POTX/POTY pins.
 */
enum {
    POT_MODE_DISCHARGE = 0,
    POT_MODE_READ
};


/* Update the USB mouse report values to be sent. Returns true if values
 * changed since last called.
 */
void updateUsbMouse()
{
    auto x = c1351.getVelocityX();
    auto y = c1351.getVelocityY();
    uint8_t lbutton = c1351.getLeftButtonValue();
    uint8_t rbutton = c1351.getRightButtonValue();
    uint8_t buttons = lbutton | (rbutton << 1);
    setUsbMouse(x, y, buttons);
}


ISR(TIMER4_COMPA_vect)
{
    static volatile uint8_t mode = POT_MODE_DISCHARGE;
    static uint8_t ticker = 0;

    if (mode == POT_MODE_DISCHARGE) {
        c1351.setModeSync();

        ticker = (ticker + 1) % 40;
        if (ticker == 0) {
            c1351.update();
            updateUsbMouse();
            handleUsb();
        }

        mode = POT_MODE_READ;
    }
    else {  // POT_MODE_READ
        c1351.setModeRead();

        mode = POT_MODE_DISCHARGE;
    }
    //HID_Device_USBTask(&Mouse_HID_Interface);
    //USB_USBTask();
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
    //DDR(PORT_DEBUG) |= _BV(PIN_DEBUG);
}


/* Main program entry point. */
int main(void)
{
    setupUsbMouse();
    setupIO();
    c1351.init();
    setupMainInterrupt(MAIN_INTERRUPT_INTERVAL_US);

    for (;;) {
        // For now, this happens in the interrupt, though we may want to move it here
        //handleUsb();
    }

    return 0;
}

