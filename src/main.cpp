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


const int MAIN_INTERRUPT_INTERVAL_US = 256;
// Update the USB mouse velocity after this many C1351 read cycles
const uint8_t MOUSE_UPDATE_INTERVAL = 40;


using c1351_mouse::C1351Interface;


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
    static uint8_t mode = POT_MODE_DISCHARGE;
    static uint8_t interval_counter = MOUSE_UPDATE_INTERVAL;

    if (mode == POT_MODE_DISCHARGE) {
        c1351.setModeSync();

        if (--interval_counter == 0) {
            interval_counter = MOUSE_UPDATE_INTERVAL;
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
}


/* Setup main control interrupt (Timer 4 compare A) */
void setupMainInterrupt(int interval_us)
{
    const int PRESCALE = 16;
    // compare match register duration (desired # cycles minus 1)
    const int OCR_COMPARE_VALUE = (F_CPU / 1000000.0 / PRESCALE) * interval_us - 1;
    cli();
    TCCR4A = 0;
    TCCR4B = 0;
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;
    TCNT4  = 0;
    OCR4A = OCR_COMPARE_VALUE;
    TCCR4B |= _BV(CS42) | _BV(CS40);    // 16x prescale
    TIMSK4 |= _BV(OCIE4A);  // enable timer compare interrupt
    sei();
}


/* Set all pins to input with internal pullup to ensure defined levels
 * and reduce power consumption for unused pins.
 */
void clearIO()
{
    PortB::setDirection(0);
    PortB::setPins(0xff);
    PortC::setDirection(0);
    PortC::setPins(0xff);
    PortD::setDirection(0);
    PortD::setPins(0xff);
    PortE::setDirection(0);
    PortE::setPins(0xff);
    PortF::setDirection(0);
    PortF::setPins(0xff);
}


/* Main program entry point. */
int main(void)
{
    clearIO();
    setupUsbMouse();
    c1351.init();
    setupMainInterrupt(MAIN_INTERRUPT_INTERVAL_US);

    for (;;) {
    }

    return 0;
}

