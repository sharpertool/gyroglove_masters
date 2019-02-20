                                                 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "SPI_sw_adc.h"

SPI_sw_adc::SPI_sw_adc(
        PORT_t* port,
        uint8_t sck_pin,
        uint8_t miso_pin,
        uint8_t mosi_pin,
        uint8_t busy_pin,
        uint8_t dataOrder
        )
{
    _port       = port;
    _sck_pin    = sck_pin;
    _miso_pin   = miso_pin;
    _mosi_pin   = mosi_pin;
    _busy_pin   = busy_pin;
    
    _dataOrder  = dataOrder;
    _pBuffer    = 0;
    _pRxBuffer  = 0;
    _bufferPos  = 0;
    _bufferSize = 0;
    _nCharsToSend = 0;
    _nRxChars       = 0;
}

SPI_sw_adc::~SPI_sw_adc()
{
    end();
}

int SPI_sw_adc::begin()
{
    _port->OUTCLR = _mosi_pin | _sck_pin;
    _port->DIRCLR = _miso_pin | _busy_pin;
    _port->DIRSET = _sck_pin | _mosi_pin; // SCK, MOSI, nCS
    return 0;
}

void SPI_sw_adc::end()
{
    if (_pBuffer) {
        free(_pBuffer);
        _pBuffer = 0;
    }
    if (_pRxBuffer) {
        free(_pRxBuffer);
        _pRxBuffer = 0;
    }
}

size_t SPI_sw_adc::getdata(uint8_t* pBuffer, size_t maxBytes)
{
    size_t nRet = maxBytes < _nRxChars ? maxBytes : _nRxChars; 
    memcpy(pBuffer,_pRxBuffer,nRet);
    return nRet;
}

// Copy the bytes to be sent into the send buffer, then
// start sending a byte at a time.
void SPI_sw_adc::send(uint8_t* pBuffer, size_t nBytes)
{
    // Verify that there is enough space in the Rx and
    // Tx buffers. Reallocate them if they are too small.
    uint8_t* pCur = pBuffer;
    if (_pRxBuffer && _bufferSize < nBytes) {
        free(_pRxBuffer);
        _pRxBuffer = 0;
    }
    
    // Reallocat teh buffers if they are not already available
    if (_pRxBuffer == 0) {
        _pRxBuffer = (uint8_t*)malloc(nBytes);
        _bufferSize = nBytes;
    }
        
    _nRxChars = 0;

    uint8_t nSent = 0;
    
    // If nothing to send, then don't bother..
    if (nBytes == 0) return;

    // Send the first byte..
    _pRxBuffer[nSent++] = sendbyte(*pCur);
    ++pCur;
    _delay_us(10);
    //uint8_t busyWaitTO = 20;
    //while (--busyWaitTO && isBusy()) {
    //    _delay_us(1);
    //}
    
    while (nSent < nBytes) {
        _pRxBuffer[nSent++] = sendbyte(*pCur);
        ++pCur;
        _delay_us(10);
    }
    _nRxChars = nSent;
}

//! Check if BUSY is high
//! Busy == 1 indicates that the ADC is busy..
bool SPI_sw_adc::isBusy()
{
    if ((_port->IN & _busy_pin) == 0)
        return false;
    
    return true;
}

uint8_t SPI_sw_adc::sendbyte(uint8_t byte)
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

uint8_t SPI_sw_adc::sendbit(uint8_t b)
{
    if (b) 
        _port->OUTSET = _mosi_pin;
    else
        _port->OUTCLR = _mosi_pin;
    
    _port->OUTSET = _sck_pin;
    uint8_t inVal = _port->IN;
    _port->OUTCLR = _sck_pin;
    _port->OUTCLR = _mosi_pin;

    return (inVal & _miso_pin) ? 1 : 0;
}

void SPI_sw_adc::setBitOrder(uint8_t t)
{
    _dataOrder = t;    
}

void SPI_sw_adc::setDataMode(uint8_t m)
{
}

void SPI_sw_adc::setClockDivider(uint8_t div)
{
}



