#ifndef DEBUGPORT_H
#define DEBUGPORT_H

#include "Port.h"

class DebugPort : public Port
{
public:
    
    uint8_t _pinMask;
    uint8_t _lastState;
    uint8_t _nShift;

    DebugPort(PORT_t* pPort);
    
    void PinMask(uint8_t mask,uint8_t shift);

    //! Update masked pins with new state
    //! The masked pins may not be sequential, so we
    //! iterate over the pinMask using tMask.
    //! bitMask selects succesive bits in s to output
    //! We drive all pins low first, then drive the new
    //! state.
    void SetState(uint8_t s);
};

#endif
