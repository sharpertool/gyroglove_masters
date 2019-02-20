
#include <avr/io.h>
#include "DebugPort.h"

DebugPort::DebugPort(PORT_t* pPort) : Port(pPort)
{
    _pinMask = 0;
    _lastState = 0;
    _nShift = 0;
}

void DebugPort::PinMask(uint8_t mask, uint8_t shift)
{
    _pinMask = mask;
    _nShift = shift;
    PinControl(mask,
        false, false,
        PORT_OPC_TOTEM_gc,
        PORT_ISC_BOTHEDGES_gc);
    
    SetPinsAsOutput(_pinMask);
    SetPinsLow(_pinMask);
    _lastState = 0;
}

//! Update masked pins with new state
//! The masked pins may not be sequential, so we
//! iterate over the pinMask using tMask.
//! bitMask selects succesive bits in s to output
//! We drive all pins low first, then drive the new
//! state.
void DebugPort::SetState(uint8_t s)
{
    _pPort->OUT = (_pPort->OUT & ~_pinMask) 
            | ((s << _nShift) & _pinMask);
    _lastState = s;
}

