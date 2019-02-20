
#ifndef SPI_i2c_h
#define SPI_i2c_h

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include "SPI.h"
#include "I2C_Master.h"

class SPI_i2c : public SPI
{
private:
    I2C_Master*     _pMaster;
    uint8_t         _ID;
    uint8_t*        _pBuffer;
    uint8_t         _bufferSize;
    uint8_t         _nRxChars;
    uint8_t         _dataOrder;
    uint8_t         _spiMode;
    uint8_t         _clkRate;

public:
    SPI_i2c(
        I2C_Master* pMaster,
        uint8_t     ID
        );
    ~SPI_i2c();
    
    void send(uint8_t* pBuffer, size_t nBytes);
    size_t getdata(uint8_t* pBuffer, size_t maxBytes);
    int begin();
    void end();
    
    void setBitOrder(uint8_t);
    void setDataMode(uint8_t);
    void setClockDivider(uint8_t);
private:
    void configure();
};


#endif

