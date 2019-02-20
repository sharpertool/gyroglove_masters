
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "SPI_hw.h"

/*! \class SPI_hw
    \def SPI_ISR_DEF(periph)

    \brief Generate ISR handlers for SPI interupts.
    
    hook them up to a class if/when a class 
    is instantiated for a particular SPI port. The class will connect it's "this"
    pointer to the static SPI_hw pointer that matches the SPI_t pointer passed into
    the class. It is important that only one class for each SPI_t pointer be
    constructed. 
*/
#define SPI_ISR_DEF(periph) \
static SPI_hw*  periph##cp = 0;\
ISR(periph##_INT_vect) {\
    if (periph##cp) periph##cp->spi_int();\
}

/*! \class SPI_hw
    Call the macro defined above for each of the SPI* types declared for this device.
    These types, i.e. SPIC, SPID, are defined in the io**.h file that is associated
    with the current ATXMega device type. 
*/
SPI_ISR_DEF(SPIC);
SPI_ISR_DEF(SPID);
SPI_ISR_DEF(SPIE);
#if defined(SPIF_INT_vect)
SPI_ISR_DEF(SPIF);
#endif


//! Get the last read byte and start transmission of next byte
/*!
    Always copy the last read byte into the RxBuffer. The _nRxChars
    will point to the next available slot.
    
    If there are more bytes to send, then decrement the nCharsToSend
    counter and send the next byte in the buffer.
*/
void SPI_hw::spi_int()
{
    _pRxBuffer[_nRxChars++] = _spi->DATA;
    if (_nCharsToSend) {
        --_nCharsToSend;
        _spi->DATA = _pBuffer[_bufferPos++];
    }
}

//! Construct the SPI_hw object
/*!
    \param[in] spi Pointer to SPI_t data structure to use.
    \param[in] clk2x 1 => double the Clk frequency in master mode.
    \param[in] dataOrder 1 => Transmit LSB first, 0 => Transmit MSB First.
    \param[in] enMaster 1=> Master mode selected, 0 => slave mode.
    \param[in] spiMode The mode for the SPI Engine to use. See the following table:<p>
    <table border="1">
        <tr>
            <th>Mode[1:0]</th>
            <th>Group Configuration</th>
            <th>Leading Edge</th>
            <th>Trailing Edge</th>
        </tr>
        <tr>
            <td>00</td>
            <td>0</td>
            <td>Rising, Sample</td>
            <td>Falling, Setup</td>
        </tr>
        <tr>
            <td>01</td>
            <td>1</td>
            <td>Rising, Setup</td>
            <td> Falling, Sample</td>
        </tr>
        <tr>
            <td>10</td>
            <td>2</td>
            <td>Falling,Sample</td>
            <td>Rising, Setup</td>
        </tr>
        <tr>
            <td>11</td>
            <td>3</td>
            <td>Falling, Setup</td>
            <td>Rising, Sample</td>
        </tr>
    </table>
    \param[in] preScale Two bit value that sets the prescale value. The meaning
    of these bits depends on the value of clk2X. The following table shows the results
    for all possible values of the prescaler with and without clk2X set to 1.
    :<p>
    
<table border="1">
  <tr>
    <td>CLK2X </td>
    <td>PRESCALER[1:0] </td>
    <td>SCK Frequency </td>
  </tr>
  <tr>
    <td>0</td>
    <td>00</td>
    <td>clkPER/4 </td>
  </tr>
  <tr>
    <td>0</td>
    <td>01</td>
    <td>clkPER/16 </td>
  </tr>
  <tr>
    <td>0</td>
    <td>10</td>
    <td>clkPER/64 </td>
  </tr>
  <tr>
    <td>0</td>
    <td>11</td>
    <td>clkPER/128 </td>
  </tr>
  <tr>
    <td>1</td>
    <td>00</td>
    <td>clkPER/2 </td>
  </tr>
  <tr>
    <td>1</td>
    <td>01</td>
    <td>clkPER/8 </td>
  </tr>
  <tr>
    <td>1</td>
    <td>10</td>
    <td>clkPER/32 </td>
  </tr>
  <tr>
    <td>1</td>
    <td>11</td>
    <td>clkPER/64 </td>
  </tr>
</table>

    The constructor will copy all of the parameters passed in to class variables.
    It will set the address of this class - the "this" pointer - to be the target
    for any interupts on this SPI port. This allows the member funcion, spi_int()
    to be called when interups occur for the specified SPI_t hardware port.
    
    The constructure also uses the SPI_t pointer passed in to determine the proper
    PORT_t to use. Since all SPI_t ports are mapped to a known PORT_t, there is no
    ambiguity when making this selection.

*/
SPI_hw::SPI_hw(SPI_t* spi,
        uint8_t clk2x,
        uint8_t dataOrder,
        uint8_t enMaster,
        uint8_t spiMode,
        uint8_t preScale)
{
    _spi        = spi;
    _clk2x      = clk2x;
    _dataOrder  = dataOrder;
    _enMaster   = enMaster;
    _spiMode    = spiMode;
    _preScale   = preScale;
    _pBuffer    = 0;
    _bufferSize = 0;
    _nCharsToSend = 0;
    
    PORT_t* pPort = 0;
    if (spi == &SPIC) {
        pPort = &PORTC;
    } else if (spi == &SPID) {
        pPort = &PORTD;
    } else if (spi == &SPIE) {
        pPort = &PORTE;
#if defined(SPIF_INT_vect)
    } else if (spi == &SPIF) {
        pPort = &PORTF;
#endif
    }
    _port       = pPort;

    if(spi == &SPIC) {
        SPICcp = this;
    } else if (spi == &SPID) {
        SPIDcp = this;
    } else if (spi ==  &SPIE) {
        SPIEcp = this;
#if defined(SPIF_INT_vect)
    } else if (spi ==  &SPIF) {
        SPIFcp = this;
#endif
    }
}

//! Destroy the SPI_hw object
/*!
    The end function is called automatically. Then, the SPI interupt pointer
    that was previously set to point to this class is cleared to NULL, which
    turns off the interupt callback mechanism.
*/
SPI_hw::~SPI_hw()
{
    end();

    if(_spi == &SPIC) {
        SPICcp = 0;
    } else if (_spi == &SPID) {
        SPIDcp = 0;
    } else if (_spi ==  &SPIE) {
        SPIEcp = 0;
#if defined(SPIF_INT_vect)
    } else if (_spi ==  &SPIF) {
        SPIFcp = 0;
#endif
    }
}

//! Retrieve data from the receiver buffer.
/*!
    \param[in] pBuffer pointer to destination data buffer
    \param[in] maxBytes value to indicate the maximum size of the input buffer.
    \return number of bytes copied.
    Pass in a buffer where the data in the current receiver buffer can be copied.
    Specify the size of the buffer to insure that the copy will not overrun the
    buffer.
*/
size_t SPI_hw::getdata(uint8_t* pBuffer, size_t maxBytes)
{
    size_t nRet = maxBytes < _nRxChars ? maxBytes : _nRxChars;
    memcpy(pBuffer,_pRxBuffer,nRet);
    return nRet;
}

//!
/*!
*/
int SPI_hw::begin()
{
    uint8_t ctrl =0;
    ctrl = _clk2x ? SPI_CLK2X_bm : 0 |
            SPI_ENABLE_bm |
            _dataOrder ? SPI_DORD_bm : 0 |
            _enMaster ? SPI_MASTER_bm : 0 |
            ((_spiMode << SPI_MODE_gp) & SPI_MODE_gm) | 
            ((_preScale << SPI_PRESCALER_gp) & SPI_PRESCALER_gm);
    _spi->INTCTRL = SPI_INTLVL_LO_gc;
    _spi->CTRL = ctrl;
    _port->DIRCLR = PIN6_bm;  // MISO
    _port->DIRSET = PIN7_bm | PIN5_bm | PIN4_bm; // SCK, MOSI, nCS
    return 0;
}

//! Disable the SPI engine and release buffers.
/*!
*/
void SPI_hw::end()
{
    _spi->CTRL &= ~SPI_ENABLE_bm;
    if (_pBuffer) {
        free(_pBuffer);
        _pBuffer = 0;
    }
}

//! Send data to SPI slave.
/*!
    \param[in] pBuffer uint8_t* buffer with data to send
    \param[in] nBytes 8 bit integer indicating how many bytes to send.
    Pass in a buffer containing the data to send, and a
    byte indicating how many bytes to send. The data to send
    will be copied to an internal buffer, then the hardware engine
    will be used to send all of the bytes passed in. The interupt will
    fire after each byte and the class will send the next byte until there
    are no more bytes to send. 
    
    After each byte the received data will be copied to an internal
    buffer so that any received bytes can be retrieved later.
*/
void SPI_hw::send(uint8_t* pBuffer, size_t nBytes)
{
    if (_pBuffer && _bufferSize < nBytes) {
        free(_pBuffer);
        free(_pRxBuffer);
        _pBuffer = 0;
        _pRxBuffer = 0;
    }
    
    if (_pBuffer == 0) {
        _pBuffer = (uint8_t*)malloc(nBytes);
        _pRxBuffer = (uint8_t*)malloc(nBytes);
        _bufferSize = nBytes;
    }
        
    strncpy((char*)_pBuffer,(char*)pBuffer,nBytes);
    _bufferPos = 0;
    _nCharsToSend = nBytes;
    _nRxChars = 0;
}

//! Set the bit order
/*!
    Not implemented yet. This shold just update the register.
*/
void SPI_hw::setBitOrder(uint8_t t)
{
    
}

//! Set the data mode
/*!
    Not implemented yet. This shold just update the register.
*/
void SPI_hw::setDataMode(uint8_t m)
{
}

//! Set the clock divider
/*!
    Not implemented yet. This shold just update the register.
*/
void SPI_hw::setClockDivider(uint8_t div)
{
}



