#include <avr/interrupt.h>
#include <avr/io.h>

#include "controller.hpp"
#include "pins.hpp"


volatile PotValue potXValue;
volatile PotValue potYValue;


void c1351PotsOutputLow()
{
    PORT(PORT_POTY) &= ~_BV(PIN_POTY);
    DDR(PORT_POTY) |= _BV(PIN_POTY);

    PORT(PORT_BTN2_POTX) &= ~_BV(PIN_BTN2_POTX);
    DDR(PORT_BTN2_POTX) |= _BV(PIN_BTN2_POTX);
}


void c1351PotsInput()
{
    DDR(PORT_POTY) &= ~_BV(PIN_POTY);
    DDR(PORT_BTN2_POTX) &= ~_BV(PIN_BTN2_POTX);
}


void c1351Init()
{
    potXValue = 0;
    potYValue = 0;
    c1351PotsOutputLow();
}


void c1351Sync()
{
    c1351PotsOutputLow();
}


void c1351Read()
{
    c1351PotsInput();
}


PotValue getPotXValue()
{
    return potXValue;
}


PotValue getPotYValue()
{
    return potYValue;
}
