#pragma once
#ifndef CAPTURE_TIMER_HPP
#define CAPTURE_TIMER_HPP

#include <stdint.h>


enum TimerNumber {
    TIMER_1,
    TIMER_3
};


/*  Number of bits used by capture timer. Max timer value will be
    2**captureTimerBits.
*/
const uint8_t captureTimerBits = 12;

uint16_t getInputCaptureTimestamp(TimerNumber);
/* Hardware input capture on ICP1 */
void initInputCapture();
void armInputCapture();
void disarmInputCapture(TimerNumber);

#endif
