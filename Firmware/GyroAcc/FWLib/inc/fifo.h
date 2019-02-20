
#ifndef _fifo_h
#define _fifo_h

#include <inttypes.h>
#include "HardwareSerial.h"

/*! Fifo Class for unsigned 8 bit values.

Construct a fifo and specify the number of elements to
store. The fifo constructor will allocate memory for the
specified number of values. The Fifo class contains member
functions for pusshing, popping and checking the status
of the fifo.
*/ 
class Fifo
{
public:
    
    typedef uint8_t  FifoType;
private:
    
    FifoType*       _pdata;
    FifoType*       _start;
    FifoType*       _end;
    uint8_t         _size;
    
public:
    Fifo(uint8_t size);
    
    
    int8_t push(FifoType*);
    int8_t pop(FifoType* pData);
    uint8_t count();
    bool full();
    bool empty();
    void clear();
};

extern void FifoTest(HardwareSerial& ds);

#endif


