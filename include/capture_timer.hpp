#pragma once
#ifndef CAPTURE_TIMER_HPP
#define CAPTURE_TIMER_HPP

#include <stdint.h>


namespace c1351_mouse {

enum TimerNumber {
    TIMER_1,
    TIMER_3
};

const int CAPTURE_TIMER_PRESCALE = 1;

uint16_t getInputCaptureTimestamp(TimerNumber);
/* Hardware input capture on ICP1/ICP3 */
void initInputCapture();
void armInputCapture();
void disarmInputCapture(TimerNumber);

}
#endif
