#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "HardwareSerial.h"

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.
#define RX_BUFFER_SIZE 128

struct ring_buffer {
    unsigned char buffer[RX_BUFFER_SIZE];
    int head;
    int tail;
};

// Generate all of the ISR handlers.. hook them up to a class if/when a class
// is instantiated for a particular USART.
#define SERIAL_ISR_DEF(usart) \
static HardwareSerial*  usart##cp = 0;\
ISR(usart##_RXC_vect) {\
    if (usart##cp) usart##cp->rxc();\
}\
ISR(usart##_DRE_vect) {\
    if (usart##cp) usart##cp->dre();\
}\
ISR(usart##_TXC_vect) {\
    if (usart##cp) usart##cp->txc();\
}

SERIAL_ISR_DEF(USARTC0);
SERIAL_ISR_DEF(USARTC1);
SERIAL_ISR_DEF(USARTD0);
SERIAL_ISR_DEF(USARTD1);
SERIAL_ISR_DEF(USARTE0);
SERIAL_ISR_DEF(USARTE1);
SERIAL_ISR_DEF(USARTF0);
#if defined(USARTF1_RXC_vect)
SERIAL_ISR_DEF(USARTF1);
#endif

inline void store_char(unsigned char c, ring_buffer *rx_buffer)
{
    int i = (rx_buffer->head + 1) % RX_BUFFER_SIZE;
    
    // if we should be storing the received character into the location
    // just before the tail (meaning that the head would advance to the
    // current location of the tail), we're about to overflow the buffer
    // and so we don't write the character or advance the head.
    if (i != rx_buffer->tail) {
        rx_buffer->buffer[rx_buffer->head] = c;
        rx_buffer->head = i;
    }
}

static void SetPointer(USART_t* usart,HardwareSerial* p)
{
    // Register this object with the appropriate
    // pointer so that the ISR routines can call p
    // class.
    if(usart == &USARTC0) {
        USARTC0cp = p;
    } else if (usart == &USARTC1) {
        USARTC1cp = p;
    } else if (usart ==  &USARTD0) {
        USARTD0cp = p;
    } else if (usart ==  &USARTD1) {
        USARTD1cp = p;
    } else if (usart ==  &USARTE0) {
        USARTE0cp = p;
    } else if (usart ==  &USARTE1) {
        USARTE1cp = p;
    } else if (usart ==  &USARTF0) {
        USARTF0cp = p;
#if defined(USARTF1_RXC_vect)
    } else if (usart ==  &USARTF1) {
        USARTF1cp = p;
#endif
    }
}

/** @name Interrupt Handlers
 
There are three possible interrupts for the USART. Receive done, Transmit
done and Data Register Ready.
*/
//@{

void HardwareSerial::rxc()
{
    unsigned char c = _usart->DATA;
    store_char(c,_rx_buffer);
}

void HardwareSerial::dre()
{
}

void HardwareSerial::txc()
{
}

//@}

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(
    USART_t     *usart,
    PORT_t      *port,
    uint8_t      in_bm,
    uint8_t      out_bm)
{
    _rx_buffer = (ring_buffer*)malloc(RX_BUFFER_SIZE+2*sizeof(int));
    _usart     = usart;
    _port      = port;
    _in_bm     = in_bm;
    _out_bm    = out_bm;
    _bsel      = 0;
    _bscale    = 0;
    _baudrate  = 9600;
    _bEn       = true;
    SetPointer(_usart,this);
}

HardwareSerial::~HardwareSerial()
{
    end();
    free(_rx_buffer);
    _rx_buffer = 0;
    SetPointer(_usart,0);
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(long baud,int8_t bscale)
{
    uint16_t BSEL;
    _bscale = bscale;
    _baudrate = baud;
    
    float fPER = F_CPU;
    float fBaud = baud;
    
    _port->DIRCLR = _in_bm;  // input
    _port->DIRSET = _out_bm; // output
    
    // set the baud rate
    if (bscale >= 0) {
        BSEL = fPER/((1 << bscale) * 16 * baud) - 1;
        //BSEL = F_CPU / 16 / baud - 1;
    } else {
        bscale = -1 * bscale;
        BSEL = (1 << bscale) * (fPER/(16.0 * fBaud) - 1);
    }
    
    _usart->BAUDCTRLA = (uint8_t)BSEL;
    _usart->BAUDCTRLB = ((bscale & 0xf) << 4) | ((BSEL & 0xf00) >> 8);
    
    // enable Rx and Tx
    _usart->CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
    // enable interrupt
    _usart->CTRLA = USART_RXCINTLVL_HI_gc;
    
    // Char size, parity and stop bits: 8N1
    _usart->CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;
}

void HardwareSerial::begin2x(long baud,int8_t bscale)
{
    uint16_t baud_setting;
    _bscale = bscale;
    _baudrate = baud;
    
    // TODO: Serial. Fix serial double clock.
    long fPER = F_CPU * 4;
    
    _port->DIRCLR = _in_bm;  // input
    _port->DIRSET = _out_bm; // output
    
    // set the baud rate using the 2X calculations
    _usart->CTRLB |= 1 << 1; // the last 1 was the _u2x value
    baud_setting = fPER / 8 / baud - 1;

    _usart->BAUDCTRLA = (uint8_t)baud_setting;
    _usart->BAUDCTRLB = baud_setting >> 8;
    
    
    // enable Rx and Tx
    _usart->CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
    // enable interrupt
    _usart->CTRLA = (_usart->CTRLA & ~USART_RXCINTLVL_gm) | USART_RXCINTLVL_LO_gc;
    
    // Char size, parity and stop bits: 8N1
    _usart->CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;
    SetPointer(_usart,this);
}

void HardwareSerial::end()
{
    SetPointer(_usart,(HardwareSerial*)0);
    
    // disable Rx and Tx
    _usart->CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    // disable interrupt
    _usart->CTRLA = (_usart->CTRLA & ~USART_RXCINTLVL_gm) | USART_RXCINTLVL_LO_gc;
}

uint8_t HardwareSerial::available(void)
{
    return (RX_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % RX_BUFFER_SIZE;
}

int HardwareSerial::read(void)
{
    // if the head isn't ahead of the tail, we don't have any characters
    if (_rx_buffer->head == _rx_buffer->tail) {
        return -1;
    } else {
        unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
        _rx_buffer->tail = (_rx_buffer->tail + 1) % RX_BUFFER_SIZE;
        return c;
    }
}

void HardwareSerial::flush()
{
    // don't reverse this or there may be problems if the RX interrupt
    // occurs after reading the value of rx_buffer_head but before writing
    // the value to rx_buffer_tail; the previous value of rx_buffer_head
    // may be written to rx_buffer_tail, making it appear as if the buffer
    // were full, not empty.
    _rx_buffer->head = _rx_buffer->tail;
}

void HardwareSerial::write(uint8_t c)
{
    if (_bEn) {
        while ( !(_usart->STATUS & USART_DREIF_bm) );
        _usart->DATA = c;
    }
}

void HardwareSerial::enable(bool bEn)
{
    _bEn = bEn;
}


