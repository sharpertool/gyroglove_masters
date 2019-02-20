
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "SPI_sw.h"

SPI_sw::SPI_sw(
        PORT_t* port,
        uint8_t sck_pin,
        uint8_t miso_pin,
        uint8_t mosi_pin,
        uint8_t cs_pin,
        uint8_t dataOrder,
        uint8_t enMaster,
        uint8_t spiMode)
{
    _port       = port;
    _port       = port;
    _sck_pin    = sck_pin;
    _miso_pin   = miso_pin;
    _mosi_pin   = mosi_pin;
    _cs_pin     = cs_pin;
    
    _dataOrder  = dataOrder;
    _enMaster   = enMaster;
    _spiMode    = spiMode;
    _pBuffer    = 0;
    _bufferSize = 0;
    _nCharsToSend = 0;
    
}

SPI_sw::~SPI_sw()
{
    end();
}

int SPI_sw::begin()
{
    _port->DIRCLR = _miso_pin;  // MISO
    _port->DIRSET = _sck_pin | _mosi_pin | _cs_pin; // SCK, MOSI, nCS
    _port->OUTSET = _cs_pin;
    _port->OUTCLR = _mosi_pin | _sck_pin;
    return 0;
}

void SPI_sw::end()
{
    if (_pBuffer) {
        free(_pBuffer);
        _pBuffer = 0;
    }
}

size_t SPI_sw::getdata(uint8_t* pBuffer, size_t maxBytes)
{
    size_t nRet = maxBytes < _nRxChars ? maxBytes : _nRxChars; 
    memcpy(pBuffer,_pRxBuffer, nRet);
    return nRet;
}

// Copy the bytes to be sent into the send buffer, then
// start sending a byte at a time.
void SPI_sw::send(uint8_t* pBuffer, size_t nBytes)
{
    uint8_t* pCur = pBuffer;
    if (_pRxBuffer && _bufferSize < nBytes) {
        free(_pRxBuffer);
        _pRxBuffer = 0;
    }
    
    if (_pRxBuffer == 0) {
        _pRxBuffer = (uint8_t*)malloc(nBytes);
        _bufferSize = nBytes;
    }
        
    _nRxChars = 0;

    uint8_t i = 0;
    _port->OUTCLR = _cs_pin;
    while (i < nBytes) {
        _pRxBuffer[i++] = sendbyte(*pCur++);
    }
    _port->OUTSET = _cs_pin;
    _nRxChars = i;
}

uint8_t SPI_sw::sendbyte(uint8_t byte)
{
    uint8_t rxByte = 0;
    if (_dataOrder) { 
        // LSB First
        for (int x=0;x<8;x++) {
            rxByte |= (sendbit(byte & (1 << x)) << x);
        }
    } else {
        // MSB First, default
        for (int x=7;x>=0;x--) {
            rxByte |= (sendbit(byte & (1 << x)) << x);
        }
    }
    return rxByte;
}

uint8_t SPI_sw::sendbit(uint8_t b)
{
    if (b) 
        _port->OUTSET = _mosi_pin;
    else
        _port->OUTCLR = _mosi_pin;
    
    _port->OUTSET = _sck_pin;
    _port->OUTCLR = _sck_pin;
    
    return (_port->IN & _miso_pin) ? 1 : 0;
}

void SPI_sw::setBitOrder(uint8_t t)
{
    _dataOrder = t;    
}

void SPI_sw::setDataMode(uint8_t m)
{
    _spiMode = m;
}

void SPI_sw::setClockDivider(uint8_t div)
{
}



