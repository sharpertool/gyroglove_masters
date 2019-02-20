
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "SPI_i2c.h"

SPI_i2c::SPI_i2c(
        I2C_Master* pMaster,
        uint8_t     ID
    )
{
    _pMaster        = pMaster;
    _ID             = ID;
    _nRxChars       = 0;
    _pBuffer        = 0;
    _bufferSize     = 0;
    _dataOrder      = 0;
    _spiMode        = 0;
    _clkRate        = 1;
}                   

SPI_i2c::~SPI_i2c()
{
    end();
}

//! Initialize the SPI session.
int SPI_i2c::begin()
{
    _pBuffer = (uint8_t*)malloc(11);
    _bufferSize = 10;
    //_pMaster->begin(400e3);
    configure();
    return 0;
}

//! Terminate the SPI session. Close buffers, etc.
void SPI_i2c::end()
{
    _pMaster->end();
    
    if (_pBuffer) {
        free(_pBuffer);
        _pBuffer    = 0;
        _bufferSize = 0;
    }
}

//! Return data buffered from last operation.
size_t SPI_i2c::getdata(uint8_t* pBuffer, size_t maxBytes)
{
    int nRetries = 20;
    // Read back the results, no function ID required, and
    // no address required, so dummy write data.
    int retc = _pMaster->WriteReadSync(_ID, _pBuffer, 0, maxBytes);
    while (--nRetries && (retc == I2C_Master::eNack) ) {
        retc = _pMaster->WriteReadSync(_ID, _pBuffer, 0, maxBytes);
    }
    _pMaster->ReadData(_pBuffer,maxBytes);    
    _nRxChars = maxBytes;

    size_t nRet = maxBytes < _nRxChars ? maxBytes : _nRxChars; 
    memcpy(pBuffer,_pBuffer,nRet);
    return nRet;
}

//! Send data to the I2C->SPI Bridge
//! The Function ID is hardcoded for the CFA Controller.
//! This class could be more generic if that was fixed and turned
//! into a parameter of some type.
//! This function will update the buffer size if needed. Then, it will
//! copy data into the buffer and prefix the buffer with the function
//! id. Then, the buffer will be sent to the I2C device. Finally,
//! the result data is read out. I could make the result data read
//! occur ONLY if the data is requrested, which would be more efficient
//! in the case where writing is common but reading the results
//! is rare.
//! Update: I changed this so that reading is only done if results are
//! requested. This seemed like the more common case.
void SPI_i2c::send(uint8_t* pBuffer, size_t nBytes)
{
    if (_pBuffer && _bufferSize < (nBytes+1)) {
        free(_pBuffer);
        _pBuffer = 0;
    }
    
    if (_pBuffer == 0) {
        _pBuffer = (uint8_t*)malloc(nBytes+1);              
        _bufferSize = nBytes+1;
    }
        
    _nRxChars = 0;
    _pBuffer[0] = 0x1; // This is the function ID - Device 0
    memcpy(&_pBuffer[1],pBuffer,nBytes);

    // Write the Function ID then the data.
    _pMaster->WriteSync(_ID, _pBuffer,nBytes+1);
}

void SPI_i2c::configure()
{
    uint8_t buffer[2];
    buffer[0] = 0xF0; // Configuration Function ID.
    buffer[1] = 0;
    if (_dataOrder) {
        buffer[1] |= 0x20;
    }
    buffer[1] |= (_spiMode & 0x3) << 2;
    buffer[1] |= _clkRate & 0x3;
    _pMaster->WriteSync(_ID, &buffer[0],2);
}

void SPI_i2c::setBitOrder(uint8_t t)
{
    _dataOrder = t;    
}

void SPI_i2c::setDataMode(uint8_t m)
{
    _spiMode = m;
}

void SPI_i2c::setClockDivider(uint8_t div)
{
    _clkRate = div;
}



