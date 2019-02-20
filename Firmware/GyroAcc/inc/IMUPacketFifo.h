
#ifndef _imupacketfifo_h
#define _imupacketfifo_h

#include <inttypes.h>
#include "IMUPacket.h"
#include "HardwareSerial.h"

/*! Fifo Class for unsigned 8 bit values.

Construct a fifo and specify the number of elements to
store. The fifo constructor will allocate memory for the
specified number of values. The Fifo class contains member
functions for pusshing, popping and checking the status
of the fifo.
*/ 
class IMUPacketFifo
{
    IMUPacket*      _pdata;
    IMUPacket*      _start;
    IMUPacket*      _end;
    uint8_t         _size;
    
public:
    IMUPacketFifo(uint8_t size);
    
    
    int8_t push(const IMUPacket& rPkt);
    int8_t push(IMUPacket* pPkt);
    int8_t pop(IMUPacket**);
    uint8_t count();
    bool full();
    bool empty();
    void clear();
};

extern void FifoTest(HardwareSerial& ds);

#endif


