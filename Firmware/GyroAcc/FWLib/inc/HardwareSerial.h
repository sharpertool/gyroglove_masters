#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <avr/io.h>
#include <inttypes.h>

#include "Print.h"

struct ring_buffer;

//! HardwareSerial implementation
/*!
    This class was originally copied form the Arduino source directory
    but has been modified somewhat to customize it for the CFA project.
    
    Ths class wraps the hardware serial resource in the ATXmega
    The class handles an interupt driven receive with a fixed size
    receive buffer of 128 bytes. The current implementation uses
    a synchronous send, but a buffered send would be a great
    enhancement for performance purposes.
    
*/
class HardwareSerial : public Print
{
  protected:
    ring_buffer *_rx_buffer;
    USART_t     *_usart;
    PORT_t      *_port;
    uint8_t     _in_bm;
    uint8_t     _out_bm;
    uint8_t     _bsel;
    int8_t     _bscale;
    long        _baudrate;
    bool        _bEn;
  public:
    HardwareSerial(
        USART_t     *usart,
        PORT_t      *port,
        uint8_t     in_bm,
        uint8_t     out_bm);
    ~HardwareSerial();
    void begin(long baudrate,int8_t bscale = 0);
    void begin2x(long baudrate,int8_t bscale = 0);
    void end();
    uint8_t available(void);
    int read(void);
    void flush(void);
    virtual void write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    void rxc();
    void dre();
    void txc();
    void enable(bool bEn);
};


#endif
