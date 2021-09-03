/* Precise timing of events using hardware input capture pins and
 * timers 1 and 3
 *
 * To use, call initInputCapture(in_pin) in setup to initialize the pin
 * to be used for input capture.
 *
 * To get ready to capture a timing event, call armInputCapture()
 * Check isReady() to see if the capture is ready, then call getInputCaptureTimestamp()
 * to get the value.
 *
 * Capture is disarmed automatically after each successful capture.
 * To disarm manually, call disarmInputCapture()
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include "capture_timer.hpp"


volatile uint16_t overflowsTimer1 = 0;
volatile uint16_t timestampTimer1 = 0;
volatile uint16_t overflowsTimer3 = 0;
volatile uint16_t timestampTimer3 = 0;


uint16_t getInputCaptureTimestamp(TimerNumber n) {
    if (n == TIMER_1) {
        return timestampTimer1;
    } else {
        return timestampTimer3;
    }
}


ISR(TIMER1_OVF_vect)
{
    overflowsTimer1++;
}


ISR(TIMER1_CAPT_vect)
{
    // input capture flag is automatically cleared when this interrupt
    // is executed

    disarmInputCapture(TIMER_1);

    timestampTimer1 = ICR1;
}


ISR(TIMER3_OVF_vect)
{
    overflowsTimer3++;
}


ISR(TIMER3_CAPT_vect)
{
    // input capture flag is automatically cleared when this interrupt
    // is executed

    disarmInputCapture(TIMER_3);

    timestampTimer3 = ICR3;
}


void startTimer1()
{
    TCCR1A = 0;
    TCCR1B = _BV(ICNC1) | _BV(ICES1) | _BV(CS10);  // Timer 1, input capture on rising edge, no prescaling, noise cancellation
    //TCCR1B = _BV(ICES1) | _BV(CS10);  // Timer 1, input capture on rising edge, no prescaling, no noise cancellation
    //TCCR1B = _BV(ICES1) | _BV(CS12) | _BV(CS10);  // Timer 1, input capture on rising edge, 1024x prescaling
}


void startTimer3()
{
    TCCR3A = 0;
    TCCR3B = _BV(ICNC3) | _BV(ICES3) | _BV(CS30);  // Timer 3, input capture on rising edge, no prescaling, noise cancellation
}


void stopTimer(TimerNumber n)
{
    if (n == TIMER_1) {
        TCCR1B = 0;
    } else {
        TCCR3B = 0;
    }
}


/* Reset timer 1 registers. */
void initTimer1()
{
    TCCR1A = 0;
    TCCR1B = 0;
}


/* Reset timer 1 registers. */
void initTimer3()
{
    TCCR3A = 0;
    TCCR3B = 0;
}

/* Hardware input capture on ICP1 */
void initInputCapture()
{
    initTimer1();
    initTimer3();

    // clear input capture flags
    TIFR1 |= _BV(ICF1);
    TIFR3 |= _BV(ICF3);
}


void resetCapture1()
{
    TCNT1 = 0;
    TIMSK1 |= _BV(ICIE1) | _BV(TOIE1);  // enable capture interrupt, overflow interrupt
    TIFR1 |= _BV(ICF1);    // clear input capture flag
}


void resetCapture3()
{
    TCNT3 = 0;
    TIMSK3 |= _BV(ICIE3) | _BV(TOIE3);  // enable capture interrupt, overflow interrupt
    TIFR3 |= _BV(ICF3);    // clear input capture flag
}


void armInputCapture()
{
    //cli();
    stopTimer(TIMER_1);
    stopTimer(TIMER_3);

    resetCapture1();
    resetCapture3();

    startTimer1();
    startTimer3();
    //sei();
}


void disarmInputCapture(TimerNumber n)
{
    if (n == TIMER_1) {
        TIMSK1 &= ~_BV(ICIE1);  // disable capture interrupt
    } else {
        TIMSK3 &= ~_BV(ICIE3);  // disable capture interrupt
    }
    stopTimer(n);
}
