#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

#include "iopin.hpp"


namespace c1351_mouse {

typedef uint16_t PotValue;
typedef int8_t MouseVelocity;


/*
 *   IO pin definition for the C1351 interface.
 *
 *   ICP1 and ICP3 are "input capture" pins, and are required for proportional
 *   mode. The ATMega32u4 has these pins, but not all 32u4-based development
 *   boards expose these pins. As of this writing, the Adafruit ItsyBitsy32u4
 *   is the only small form-factor board that exposes both pins.
 *
 *   NOTE: the ItsyBitsy pin 13 is tied to the on-board LED, which needs to be
 *   physically removed for the circuit to work properly.
 *
 *  DB9 (C1351)   ItsyBitsy pin   Function     Prop. mode    Joy. mode
 * ------------- --------------- ----------- -------------- -----------
 *            1   2 (SDA)         GPIO        Rt btn         Up
 *            2   SCK             GPIO        -              Down
 *            3   8               GPIO        -              Left
 *            4   6               GPIO        -              Right
 *            5   13              ICP3        Y pos (POTY)   -
 *            6   3 (SCL)         GPIO        Left btn       Left btn
 *            7   Vcc                         5V             5V
 *            8   GND                         GND            GND
 *            9   4               ICP1/GPIO   X pos (POTX)   Right btn
 */
struct C1351_IO {
    Pin<PortB, 1> down;      // ItsyBitsy pin SCK
    Pin<PortB, 4> left;      // ItsyBitsy pin 8
    Pin<PortB, 6> debug;     // ItsyBitsy pin 10

    Pin<PortC, 7> poty;      // ItsyBitsy pin 13, ICP3

    Pin<PortD, 0> btn1;      // ItsyBitsy pin 3 (SCL)
    Pin<PortD, 1> up_btn2;   // ItsyBitsy pin 2 (SDA)
    Pin<PortD, 4> btn2_potx; // ItsyBitsy pin 4, ICP1
    Pin<PortD, 7> right;     // ItsyBitsy pin 6
};


/*  C1351 mouse interface.
 *
 *  setModeSync() and setModeRead() are used to read X and Y positions of the mouse.
 *
 *  Every 512uS, use the following sequence:
 *  - Call setModeSync(). This initiates a read cycle on the C1351 and
 *    starts a timer that will stop when the POTX and POTY lines of the C1351
 *    go high
 *  - Wait about 256uS
 *  - Call setModeRead(). This puts the POTX and POTY pins into input mode.
 *    As each pin is driven high by the C1351, its respective timer stops.
 *    The time elapsed since calling setModeSync() is used to determine the
 *    X and Y position.
 *
 * In order to ensure accurate readings, the input capture pins are used, which
 * allow timestamping via pure hardware. Thus, the readings are not affected by
 * program execution time or interrupts such as the USB interrupt.
 * For this reason, the microcontroller must have at least two ICP pins. This
 * implementation uses ICP1 and ICP3, which use timer 1 and timer 3,
 * respectively.
*/
class C1351Interface {

public:
    C1351Interface();

    /* Call once at the beginning of the program to initialize. */
    void init();
    /* Synchronize the C1351 and initiate a read cycle, start capture timers. */
    void setModeSync();
    /* Prepare for input capture event. */
    void setModeRead();
    /* Call regularly (every 20ms or so) to update mouse state. Then use
     * getVelocityX, getVelocityY, getLeftButtonValue,
     * and getRightButtonValue
     */
    void update();

    MouseVelocity getVelocityX() const;
    MouseVelocity getVelocityY() const;
    bool getLeftButtonValue() const;
    bool getRightButtonValue() const;

protected:
    C1351_IO io_pin;

    volatile PotValue potXValue = 0;
    volatile PotValue potYValue = 0;
    volatile PotValue potXValueOld = 0;
    volatile PotValue potYValueOld = 0;
    volatile MouseVelocity velocityX = 0;
    volatile MouseVelocity velocityY = 0;
    volatile int16_t velocityAccumX = 0;
    volatile int16_t velocityAccumY = 0;
    volatile bool buttonLeftPressed = false;
    volatile bool buttonRightPressed = false;

    void updatePotValues();
    void accumulateVelocities();
    void setPotsOutputLow();
    void setPotsInput();
    void initIO();

    void updateButtons();

    int16_t potValueToVelocity(PotValue oldVal, PotValue newVal);

};

}
#endif
