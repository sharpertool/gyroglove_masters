
#ifndef SPI_h
#define SPI_h

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>

class SPI
{
public:
    
    virtual void send(uint8_t* pBuffer, size_t nBytes) = 0;
    virtual size_t getdata(uint8_t* pBuffer, size_t maxBytes) = 0;
    virtual int begin() = 0;
    virtual void end() = 0;
    
    virtual void setBitOrder(uint8_t) = 0;
    virtual void setDataMode(uint8_t) = 0;
    virtual void setClockDivider(uint8_t) = 0;
};


#endif

