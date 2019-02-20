
#ifndef SPI_hw_h
#define SPI_hw_h

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include "SPI.h"

class SPI_hw: public SPI
{
private:
    SPI_t*  _spi;
    PORT_t* _port;
    uint8_t _clk2x;
    uint8_t _dataOrder;
    uint8_t _enMaster;
    uint8_t _spiMode;
    uint8_t _preScale;
    uint8_t* _pBuffer;
    uint8_t* _pRxBuffer;
    uint8_t  _bufferPos;
    uint8_t  _bufferSize;
    uint8_t  _nCharsToSend;
    uint8_t  _nRxChars;

public:
    SPI_hw(SPI_t*  spi,
        uint8_t clk2x,
        uint8_t dataOrder,
        uint8_t enMaster,
        uint8_t spiMode,
        uint8_t preScale
        );
    ~SPI_hw();
    
    void send(uint8_t* pBuffer, size_t nBytes);
    size_t getdata(uint8_t* pBuffer, size_t maxBytes);
    int begin();
    void end();
    
    void setBitOrder(uint8_t);
    void setDataMode(uint8_t);
    void setClockDivider(uint8_t);
    void spi_int();
};


#endif

