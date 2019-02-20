
#ifndef SPI_sw_adc_h
#define SPI_sw_adc_h

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include "SPI.h"

//! Software SPI class specific to TI ADC
/*!
    The TI ADS8345 connected to the CFA Controller board only has the
    Clk, DIN DOUT and Busy pins available. The CS* pin is not available. 
    Consequently, there is no hardware way to reset the ADC state engine. 
    
    The ADC state engine only uses 25 clock cycles, and it must always start
    with a '1' bit on the data line, which is the 'Start' bit. This makes it
    relatively easy to synchronize the engine. A transaction will clock out
    32 bits to be safe. The last bits will be zeros, which will clear the engine
    so that the next '1' bit, at the start of a transaction, will always be
    the first bit clocked in.
*/
class SPI_sw_adc : public SPI
{
private:
    PORT_t*     _port;
    uint8_t     _sck_pin;
    uint8_t     _miso_pin;
    uint8_t     _mosi_pin;
    uint8_t     _busy_pin;

    uint8_t     _dataOrder;
    uint8_t*    _pBuffer;
    uint8_t*    _pRxBuffer;
    uint8_t     _bufferPos;
    uint8_t     _bufferSize;
    uint8_t     _nCharsToSend;
    uint8_t     _nRxChars;
                 
public:
    SPI_sw_adc(
        PORT_t* port,
        uint8_t sck_pin,
        uint8_t miso_pin,
        uint8_t mosi_pin,
        uint8_t busy_pin,
        uint8_t dataOrder
        );
    
    ~SPI_sw_adc();
    
    void send(uint8_t* pBuffer, size_t nBytes);
    size_t getdata(uint8_t* pBuffer, size_t maxBytes);
    int begin();
    void end();
    
    bool isBusy();
    
    void setBitOrder(uint8_t);
private:
    void setClockDivider(uint8_t);
    void setDataMode(uint8_t);

    uint8_t sendbyte(uint8_t byte);
    uint8_t sendbit(uint8_t bit);
};


#endif

