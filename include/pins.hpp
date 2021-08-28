/* Pin definitions.
 *
 * Defines PORT(), DDR(), and PIN() macros.
 *
 */

#pragma once

#ifndef PINS_H
#define PINS_H


#define PORT_(port) PORT ## port
#define DDR_(port)  DDR  ## port
#define PIN_(port)  PIN  ## port

#define PORT(port) PORT_(port)
#define DDR(port)  DDR_(port)
#define PIN(port)  PIN_(port)

// Debug pin
#define PORT_DEBUG     B
#define PIN_DEBUG      6

// POTX uses input capture 1 (ICP1/PD4/ADC8)
// ItsyBitsy digital pin 4
#define PORT_BTN2_POTX D
#define PIN_BTN2_POTX  4

// POTY uses input capture 3 (ICP3/PC7/CLK0/OC4A)
// ItsyBitsy digital pin 13
#define PORT_POTY      C
#define PIN_POTY       7


#endif
