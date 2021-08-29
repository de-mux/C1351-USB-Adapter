#include <avr/interrupt.h>
#include <avr/io.h>

#include "capture_timer.hpp"
#include "controller.hpp"
#include "pins.hpp"


C1351Interface::C1351Interface() : potXValue(0), potYValue(0), potXValueOld(0),
    potYValueOld(0), velocityX(0), velocityY(0), buttonLeftPressed(false),
    buttonRightPressed(false)
{
}


void C1351Interface::init()
{
    potXValue = 0;
    potYValue = 0;
    setPotsOutputLow();

    initInputCapture();
}


void C1351Interface::setModeSync()
{
    /* FIXME: move this to a routine in main */
    static uint8_t ticker = 0;

    disarmInputCapture(TIMER_1);
    disarmInputCapture(TIMER_3);

    setPotsOutputLow();

    armInputCapture();

    ticker = (ticker + 1) % 40;
    if (ticker == 0) {
        updatePotValues();
        updateVelocities();
    }
}


void C1351Interface::setModeRead()
{
    setPotsInput();
}


/* Call to update pot values with most recent input capture timestamp. */
void C1351Interface::updatePotValues()
{
    potXValueOld = potXValue;
    potYValueOld = potYValue;
    potXValue = getInputCaptureTimestamp(TIMER_1) - 16 * 256;
    potYValue = getInputCaptureTimestamp(TIMER_3) - 16 * 256;
}


/*  Call updatePotValues() before calling this to ensure they are up
    to date.
*/
void C1351Interface::updateVelocities()
{
    const auto shift_bits = captureTimerBits - potValueBits;

    velocityX = potValueToVelocity(potXValueOld, potXValue) / 16;
    velocityY = -potValueToVelocity(potYValueOld, potYValue) / 16;
}


int8_t C1351Interface::getVelocityX()
{
    return velocityX;
}


int8_t C1351Interface::getVelocityY()
{
    return velocityY;
}


PotValue C1351Interface::getPotXValue()
{
    return potXValue;
}


PotValue C1351Interface::getPotYValue()
{
    return potYValue;
}


bool C1351Interface::getLeftButtonValue()
{
    return buttonLeftPressed;
}


bool C1351Interface::getRightButtonValue()
{
    return buttonRightPressed;
}


void C1351Interface::setPotsOutputLow()
{
    PORT(PORT_POTY) &= ~_BV(PIN_POTY);
    DDR(PORT_POTY) |= _BV(PIN_POTY);

    PORT(PORT_BTN2_POTX) &= ~_BV(PIN_BTN2_POTX);
    DDR(PORT_BTN2_POTX) |= _BV(PIN_BTN2_POTX);
}


void C1351Interface::setPotsInput()
{
    DDR(PORT_POTY) &= ~_BV(PIN_POTY);
    DDR(PORT_BTN2_POTX) &= ~_BV(PIN_BTN2_POTX);
}


/* 16-bit */
int16_t C1351Interface::potValueToVelocity(PotValue oldVal, PotValue newVal)
{
    uint16_t diff = (newVal - oldVal) & 0x7ff;
    bool isPositive = diff < 0b010000000000;

    if (isPositive) {  // diff is positive
        diff = diff >> 1;
        return (int16_t)diff;
    }
    else {    // diff is negative
        diff |= 0b1111100000000000;
        if (diff != 0xffff) {
            diff = (diff >> 1) | 0b1000000000000000;
            //int8_t newDiff = 0xff - diff;
            return (int16_t)diff;
        }

        return 0;
    }
}

