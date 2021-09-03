#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

#include "iopin.hpp"


typedef uint16_t PotValue;


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
 *            1   5               GPIO        Rt btn         Up
 *            2   6               GPIO        -              Down
 *            3   7               GPIO        -              Left
 *            4   8               GPIO        -              Right
 *            5   13              ICP3        Y pos (POTY)   -
 *            6   9               GPIO        Left btn       Left btn
 *            7   Vcc                         5V             5V
 *            8   GND                         GND            GND
 *            9   4               ICP1/GPIO   X pos (POTX)   Right btn
 */
struct C1351_IO {
    Pin<PortB, 4> right;     // ItsyBitsy pin 8
    Pin<PortB, 5> btn1;      // ItsyBitsy pin 9
    Pin<PortB, 6> debug;     // ItsyBitsy pin 10

    Pin<PortC, 6> up_btn2;   // ItsyBitsy pin 5
    Pin<PortC, 7> poty;      // ItsyBitsy pin 13, ICP3

    Pin<PortD, 4> btn2_potx; // ItsyBitsy pin 4, ICP1
    Pin<PortD, 6> down;      // ItsyBitsy pin 6

    Pin<PortE, 6> left;      // ItsyBitsy pin 7
};


//template<typename IO_PIN_TYPE>
class C1351Interface {

public:
    C1351Interface();

    /* Call once at the beginning of the program to reset IO pins. */
    void init();
    /* Call to initiate a read cycle on the C1351. */
    void setModeSync();
    /*  After initiating a read cycle with setModeSync(), wait 256uS and call setModeRead()
        to put the pins into input mode and wait for inputs to go high.
        The time elapsed indicates the mouse position.
    */
    void setModeRead();
    /* Call regularly to update mouse state. Then use
     * getVelocityX, getVelocityY, getLeftButtonValue,
     * and getRightButtonValue
     */
    void update();

    int8_t getVelocityX();
    int8_t getVelocityY();
    bool getLeftButtonValue();
    bool getRightButtonValue();

protected:
    C1351_IO io_pin;

    /* Number of bits used for POT values. */
    const uint8_t potValueBits = 8;

    volatile PotValue potXValue = 0;
    volatile PotValue potYValue = 0;
    volatile PotValue potXValueOld = 0;
    volatile PotValue potYValueOld = 0;
    volatile int8_t velocityX = 0;
    volatile int8_t velocityY = 0;
    volatile int16_t velocityAccumX = 0;
    volatile int16_t velocityAccumY = 0;
    volatile bool buttonLeftPressed = false;
    volatile bool buttonRightPressed = false;

    void updatePotValues();
    void accumulateVelocities();
    void setPotsOutputLow();
    void setPotsInput();
    void initIO();
    /* Read the most recent POTX and POTY values. */
    /* TODO: remove? */
    PotValue getPotXValue();
    PotValue getPotYValue();

    void updateButtons();

    int16_t potValueToVelocity(PotValue oldVal, PotValue newVal);

};

#endif
