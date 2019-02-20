
#ifndef SPI_sw_h
#define SPI_sw_h

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include "SPI.h"

class SPI_sw : public SPI
{
private:
    PORT_t*     _port;
    uint8_t     _sck_pin;
    uint8_t     _miso_pin;
    uint8_t     _mosi_pin;
    uint8_t     _cs_pin;

    uint8_t     _dataOrder;
    uint8_t     _enMaster;
    uint8_t     _spiMode;
    uint8_t*    _pBuffer;
    uint8_t*    _pRxBuffer;
    uint8_t     _bufferPos;
    uint8_t     _bufferSize;
    uint8_t     _nCharsToSend;
    uint8_t     _nRxChars;
                 
public:
    SPI_sw(
        PORT_t* port,
        uint8_t sck_pin,
        uint8_t miso_pin,
        uint8_t mosi_pin,
        uint8_t cs_pin,
        uint8_t dataOrder,
        uint8_t enMaster,
        uint8_t spiMode
        );
    ~SPI_sw();
    
    void send(uint8_t* pBuffer, size_t nBytes);
    size_t getdata(uint8_t* pBuffer, size_t maxBytes);
    int begin();
    void end();
    
    void setBitOrder(uint8_t);
    void setDataMode(uint8_t);
private:
    void setClockDivider(uint8_t);

    uint8_t sendbyte(uint8_t byte);
    uint8_t sendbit(uint8_t bit);
};


#endif

