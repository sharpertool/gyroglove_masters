#include <stdio.h>
#include <string.h>
#include <math.h>
#include <avr/interrupt.h>

#include "NewDel.h"
#include "IMUPacketFifo.h"

//! Construct the fifo object.
//! Allocate memory for the specified number of elements
//! and set the internal value to indicate the size of the fifo.
//! Reset the start and end data points to their clear state. The
//! clear function is called to maintain consitency and insure that
//! clear() always does the right thing.
IMUPacketFifo::IMUPacketFifo(uint8_t size)
{
    _size = size;
    _pdata = (IMUPacket*)malloc(_size*sizeof(IMUPacket));
    clear();
}

//! Clear the fifo by resetting the start and end pointer.
void IMUPacketFifo::clear()
{
    _start = _end = _pdata;
}

//! Return the number of elements currently in the fifo
//! if the end and start pointers are the same then the fifo
//! is empty and count == 0.
//! If they differ, then we need to check for wrap-around in order
//! to properly determine the size. In the following examples a
//! = marks empty spots, while an x marks filled spots. <p>

//! <pre>
//!         s                       e                        
//!  =======xxxxxxxxxxxxxxxxxxxxxxxxx=======================
//! </pre>
//! In this case end > start, so count is equal to the distance between them or
//! \f$end - start\f$.<p>

//! <pre>
//!         e                       s                       
//!  xxxxxxxx=======================xxxxxxxxxxxxxxxxxxxxxxxx
//! </pre>
//! In this case end < start, so data wraps around. The total count is equal to the
//! size of the buffer, minus the number of blank spots, or
//! \f$size - (start - end)\f$.<p>
//! The total number of possible elements that can be stored is size -1, so
//! 
uint8_t IMUPacketFifo::count()
{
    
    if (_end == _start) return 0;
    if (_end > _start) {
        return _end - _start;
    }
    return _size - (_start - _end);
}

//! Return true if the fifo is full.
bool IMUPacketFifo::full()
{
    return count() == (_size-1);
}

//! Return true if the fifo is empty.
bool IMUPacketFifo::empty()
{
    return (_start == _end);
}

//! Push a new value onto the fifo. 
//! This function returns 0 if the operation succeeds, and a negative
//! value if the operation fails.
int8_t IMUPacketFifo::push(IMUPacket* pPkt) 
{
    if (full()) return -1;
    
    // Copy once, not twice on the call.
    *(_end++) = *pPkt;
    
    // Wrap the end back to the beginning.
    if ((_end - _pdata) == _size) {
        _end = _pdata;
    }

    return 0;
}

//! Push a new value onto the fifo. 
//! This function returns 0 if the operation succeeds, and a negative
//! value if the operation fails.
int8_t IMUPacketFifo::push(const IMUPacket& rPkt) 
{
    if (full()) return -1;
    
    // Copy once, not twice on the call.
    *(_end++) = rPkt;
    
    // Wrap the end back to the beginning.
    if ((_end - _pdata) == _size) {
        _end = _pdata;
    }

    return 0;
}

//! Remove the top value from the Fifo.
//! I am passing out a reference to the internal value
//! for efficiency. This assumes that the caller will use and
//! then discard the pointer quickly.
int8_t IMUPacketFifo::pop(IMUPacket** ppPkt) 
{
    if (empty()) {
        return -1;
    }
    
    // Pass out a reference.
    *ppPkt = (_start++);
    
    if ((_start - _pdata) == _size) {
        _start = _pdata;
    }
    return 0;
}


