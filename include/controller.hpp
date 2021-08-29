#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>


typedef uint16_t PotValue;


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

    int8_t getVelocityX();
    int8_t getVelocityY();
    bool getLeftButtonValue();
    bool getRightButtonValue();
protected:
    /* Number of bits used for POT values. */
    const uint8_t potValueBits = 8;

    volatile PotValue potXValue = 0;
    volatile PotValue potYValue = 0;
    volatile PotValue potXValueOld = 0;
    volatile PotValue potYValueOld = 0;
    volatile int8_t velocityX = 0;
    volatile int8_t velocityY = 0;
    volatile bool buttonLeftPressed = false;
    volatile bool buttonRightPressed = false;

    void updatePotValues();
    void updateVelocities();
    void setPotsOutputLow();
    void setPotsInput();
    /* Read the most recent POTX and POTY values. */
    /* TODO: remove? */
    PotValue getPotXValue();
    PotValue getPotYValue();

    int16_t potValueToVelocity(PotValue oldVal, PotValue newVal);
};

#endif
