#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

#include "capture_timer.hpp"
#include "controller.hpp"
#ifdef ENABLE_VIRTUAL_SERIAL
    #include "mouse.h"
#endif


static_assert(F_CPU >= 1000000, "CPU frequency must be at least 1MHz");
const int CPU_TO_US_MULTIPLIER = F_CPU / 1000000;


namespace c1351_mouse {

C1351Interface::C1351Interface() : potXValue(0), potYValue(0), potXValueOld(0),
    potYValueOld(0), velocityX(0), velocityY(0), velocityAccumX(0), velocityAccumY(0),
    buttonLeftPressed(false),
    buttonRightPressed(false)
{
}


void C1351Interface::initIO()
{
    io_pin.debug.setDirectionOut();

    io_pin.up_btn2.setDirectionIn(true);
    io_pin.down.setDirectionIn(true);
    io_pin.left.setDirectionIn(true);
    io_pin.right.setDirectionIn(true);
    io_pin.btn1.setDirectionIn(true);

    setPotsOutputLow();
}


void C1351Interface::init()
{
    initIO();

    initInputCapture();
}


void C1351Interface::setModeSync()
{
    //io_pin.debug.low();

    bool last_capture_invalid = timerRunning(TIMER_3);

    // TODO: are these necessary?
    disarmInputCapture(TIMER_1);
    disarmInputCapture(TIMER_3);

    armInputCapture();

    setPotsOutputLow();

    if (last_capture_invalid) {
        // If Timer 3 (POTY) is still running at the start of a sync, it means
        // we didn't detect a positive edge during the last sync.
        // This could happen if there is no mouse connected, or if the mouse is
        // in "joystick"/"C1350" mode.
        return;
    }
    updatePotValues();
    accumulateVelocities();

    updateButtons();
}


void C1351Interface::setModeRead()
{
    //io_pin.debug.high();
    setPotsInput();
}


/* Call to update pot values with most recent input capture timestamp. */
void C1351Interface::updatePotValues()
{
    // Subtract the first 256 uS from the captured timestamp -- this is the
    // amount of time the C1351 will be in "sync" state after its SYNC pin
    // is pulled low.
    const uint16_t TIMESTAMP_ADJUST_US = 256;
    const uint16_t TIMESTAMP_ADJUST = (F_CPU / 1000000.0) * TIMESTAMP_ADJUST_US /
                                      CAPTURE_TIMER_PRESCALE;

    potXValueOld = potXValue;
    potYValueOld = potYValue;
    potXValue = getInputCaptureTimestamp(TIMER_1) - TIMESTAMP_ADJUST;
    potYValue = getInputCaptureTimestamp(TIMER_3) - TIMESTAMP_ADJUST;
}


/*  Call updatePotValues() before calling this to ensure they are up
    to date.
*/
void C1351Interface::accumulateVelocities()
{
    auto new_x_velocity = potValueToVelocity(potXValueOld, potXValue);
    auto new_y_velocity = -potValueToVelocity(potYValueOld, potYValue);

    velocityAccumX += new_x_velocity;
    velocityAccumY += new_y_velocity;
}


void C1351Interface::updateButtons()
{
    buttonLeftPressed = !io_pin.btn1.read();
    buttonRightPressed = !io_pin.up_btn2.read();
}


/*  Call regularly to update mouse state. Then use
    getVelocityX, getVelocityY, getLeftButtonValue,
    and getRightButtonValue
*/
void C1351Interface::update()
{
    velocityX = velocityAccumX / CPU_TO_US_MULTIPLIER;
    velocityY = velocityAccumY / CPU_TO_US_MULTIPLIER;
#ifdef ENABLE_VIRTUAL_SERIAL
    serialPrintNum(velocityX);
#endif
    velocityAccumX = 0;
    velocityAccumY = 0;
}


MouseVelocity C1351Interface::getVelocityX() const
{
    return velocityX;
}


MouseVelocity C1351Interface::getVelocityY() const
{
    return velocityY;
}


bool C1351Interface::getLeftButtonValue() const
{
    return buttonLeftPressed;
}


bool C1351Interface::getRightButtonValue() const
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

}
