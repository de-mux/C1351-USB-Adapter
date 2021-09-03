//*****************************************************************************
//
// iopin.h  - AVR pin definition template
//
// Copyright(C) 2012 Yuuichi Akagawa
//


/* This code was inspired by the Konstantin Chizhov's AVR port templates  */

#pragma once
#ifndef IOPIN_H
#define IOPIN_H

#include <avr/io.h>


//Port definitions for ATTiny, ATMega families.
#define DECLARE_PORT(CLASS_NAME, PORT_NAME, DDR_NAME, PIN_NAME) \
class CLASS_NAME{\
public:\
static inline uint8_t pinRead()              {return PIN_NAME;}\
static inline uint8_t read()                 {return PORT_NAME;}\
static inline void    setPins(uint8_t value)     {PORT_NAME |= value;}\
static inline void    clearPins(uint8_t value)   {PORT_NAME &= ~value;}\
static inline void    togglePins(uint8_t value) {PIN_NAME = value;}\
static inline void    write(uint8_t value)   {PORT_NAME = value;}\
static inline void    setDirectionOut(uint8_t value)  {DDR_NAME |= value;}\
static inline void    setDirectionIn(uint8_t value){DDR_NAME &= ~value;}\
static inline void    setDirection(uint8_t value){DDR_NAME = value;}\
};


template<typename PORT, uint8_t PIN>
class Pin {
public:
    static inline void high()
    {
        PORT::setPins(_BV(PIN));
    }
    static inline void low()
    {
        PORT::clearPins(_BV(PIN));
    }
    inline void setDirectionIn(bool enable_pullup=false)
    {
        PORT::setDirectionIn(_BV(PIN));
        PORT::setPins((uint8_t)enable_pullup << PIN);
    }
    static inline void setDirectionOut()
    {
        PORT::setDirectionOut(_BV(PIN));
    }
    static inline bool read()
    {
        return PORT::pinRead() & (uint8_t)(_BV(PIN));
    }
};


#ifdef PORTA
DECLARE_PORT(PortA, PORTA, DDRA, PINA)
#endif
#ifdef PORTB
DECLARE_PORT(PortB, PORTB, DDRB, PINB)
#endif
#ifdef PORTC
DECLARE_PORT(PortC, PORTC, DDRC, PINC)
#endif
#ifdef PORTD
DECLARE_PORT(PortD, PORTD, DDRD, PIND)
#endif
#ifdef PORTE
DECLARE_PORT(PortE, PORTE, DDRE, PINE)
#endif
#ifdef PORTF
DECLARE_PORT(PortF, PORTF, DDRF, PINF)
#endif
#ifdef PORTG
DECLARE_PORT(PortG, PORTG, DDRG, PING)
#endif
#ifdef PORTH
DECLARE_PORT(PortH, PORTH, DDRH, PINH)
#endif
#ifdef PORTJ
DECLARE_PORT(PortJ, PORTJ, DDRJ, PINJ)
#endif
#ifdef PORTK
DECLARE_PORT(PortK, PORTK, DDRK, PINK)
#endif
#ifdef PORTL
DECLARE_PORT(PortL, PORTL, DDRL, PINL)
#endif
#ifdef PORTQ
DECLARE_PORT(PortQ, PORTQ, DDRQ, PINQ)
#endif
#ifdef PORTR
DECLARE_PORT(PortR, PORTR, DDRR, PINR)
#endif
#endif //IOPIN_H
