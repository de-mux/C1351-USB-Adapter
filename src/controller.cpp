#include <avr/interrupt.h>
#include <avr/io.h>

#include "capture_timer.hpp"
#include "controller.hpp"
#ifdef ENABLE_VIRTUAL_SERIAL
#include "mouse.h"
#endif


C1351Interface::C1351Interface() : potXValue(0), potYValue(0), potXValueOld(0),
    potYValueOld(0), velocityX(0), velocityY(0), velocityAccumX(0), velocityAccumY(0),
    buttonLeftPressed(false),
    buttonRightPressed(false)
{
}


void C1351Interface::initIO()
{
    io_pin.debug.setDirectionOut();

    io_pin.up_btn2.setDirectionIn();
    io_pin.down.setDirectionIn();
    io_pin.left.setDirectionIn();
    io_pin.right.setDirectionIn();
    io_pin.btn1.setDirectionIn();

    setPotsOutputLow();
}


void C1351Interface::init()
{
    initIO();

    initInputCapture();
}


void C1351Interface::setModeSync()
{
    io_pin.debug.low();

    // TODO: are these necessary?
    disarmInputCapture(TIMER_1);
    disarmInputCapture(TIMER_3);

    armInputCapture();

    setPotsOutputLow();

    updatePotValues();
    accumulateVelocities();

    updateButtons();
}


void C1351Interface::setModeRead()
{
    io_pin.debug.high();
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
void C1351Interface::accumulateVelocities()
{
    //const auto shift_bits = captureTimerBits - potValueBits;

    //velocityX = potValueToVelocity(potXValueOld, potXValue) / 16;
    //velocityY = -potValueToVelocity(potYValueOld, potYValue) / 16;
    //PORT(PORT_DEBUG) &= ~_BV(PIN_DEBUG);
    auto new_x_velocity = potValueToVelocity(potXValueOld, potXValue);
    auto new_y_velocity = -potValueToVelocity(potYValueOld, potYValue);

    velocityAccumX += new_x_velocity; // / 16;
    velocityAccumY += new_y_velocity; // / 16;
    //PORT(PORT_DEBUG) |= _BV(PIN_DEBUG);
}


void C1351Interface::updateButtons()
{
}


/*  Call regularly to update mouse state. Then use
    getVelocityX, getVelocityY, getLeftButtonValue,
    and getRightButtonValue
*/
void C1351Interface::update()
{
#ifdef ENABLE_VIRTUAL_SERIAL
    serialPrintNum(velocityAccumX/16);
#endif
    velocityX = velocityAccumX / 16;
    velocityY = velocityAccumY / 16;
    velocityAccumX = 0;
    velocityAccumY = 0;
    //updatePotValues();
    //updateVelocities();
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
    io_pin.poty.low();
    io_pin.poty.setDirectionOut();

    io_pin.btn2_potx.low();
    io_pin.btn2_potx.setDirectionOut();
}


void C1351Interface::setPotsInput()
{
    io_pin.poty.setDirectionIn();
    io_pin.btn2_potx.setDirectionIn();
}


int16_t C1351Interface::potValueToVelocity(PotValue oldVal, PotValue newVal)
{
    uint16_t diff = (newVal - oldVal) & 0x7ff;  // 2047
    bool isPositive = diff < (1 << 10);  // 1024

    if (isPositive) {  // diff is positive
        return (int16_t)diff;
    }
    else {    // diff is negative
        diff |= 0b1111100000000000;
        return (int16_t)diff;
    }
}

