#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>


typedef uint16_t PotValue;

/* Call once at the beginning of the program to reset IO pins. */
void c1351Init();
/* Call to initiate a read cycle on the C1351. */
void c1351Sync();
/* After initiating a read cycle with c1351Sync(), wait 256uS and call c1351Read()
 * to put the pins into input mode and wait for inputs to go high.
 * The time elapsed indicates the mouse position.
 */
void c1351Read();

/* Read the most recent POTX and POTY values. */
PotValue getPotXValue();
PotValue getPotYValue();


#endif
