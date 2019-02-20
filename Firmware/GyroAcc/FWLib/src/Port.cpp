

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "Port.h"
#include "HardwareSerial.h"

extern HardwareSerial* pdbgserial;

// Generate all of the ISR handlers.. hook them up to a class if/when a class
// is instantiated for a particular USART.
#define PORT_ISR_DEF(port) \
static Port*  port##cp = 0;\
ISR(port##_INT0_vect) {\
    if (port##cp) port##cp->int0();\
}\
ISR(port##_INT1_vect) {\
    if (port##cp) port##cp->int1();\
}

PORT_ISR_DEF(PORTA);
PORT_ISR_DEF(PORTB);
PORT_ISR_DEF(PORTC);
PORT_ISR_DEF(PORTD);
PORT_ISR_DEF(PORTE);
PORT_ISR_DEF(PORTF);
#if defined (__AVR_ATxmega128A1__)
PORT_ISR_DEF(PORTH);
#endif

static void SetPointer(PORT_t* port,Port* p)
{
    // Register this object with the appropriate
    // pointer so that the ISR routines can call p
    // class.
    if(port == &PORTA) {
        PORTAcp = p;
    } else if (port == &PORTB) {
        PORTBcp = p;
    } else if (port ==  &PORTC) {
        PORTCcp = p;
    } else if (port ==  &PORTD) {
        PORTDcp = p;
    } else if (port ==  &PORTE) {
        PORTEcp = p;
    } else if (port ==  &PORTF) {
        PORTFcp = p;
#if defined (__AVR_ATxmega128A1__)
    } else if (port ==  &PORTH) {
        PORTHcp = p;
#endif
    }
}


Port::Port(PORT_t* pPort)
{
    _pPort = pPort;
    SetPointer(_pPort,this);
}

Port::~Port()
{
    SetPointer(_pPort,0);
}

void Port::Notify(PortNotify* pClient,uint8_t id)
{
    _pNotifyClient  = pClient;
    _pNotifyID      = id;
}

void Port::int0()
{
    if (_pNotifyClient) {
        _pNotifyClient->PortISR0(_pNotifyID);
    }
    _pPort->INTFLAGS = 0x1;
}

void Port::int1()
{
    if (_pNotifyClient) {
        _pNotifyClient->PortISR1(_pNotifyID);
    }
    _pPort->INTFLAGS = 0x2;
}

void Port::SetDir(uint8_t dir)
{
    _pPort->DIR = dir;
}

void Port::SetPinsAsInput(uint8_t mask)
{
    _pPort->DIRCLR = mask;
}

void Port::SetPinsAsOutput(uint8_t mask)
{
    _pPort->DIRSET = mask;
}

void Port::SetPinsHigh(uint8_t mask)
{
    _pPort->OUTSET = mask;
}

void Port::SetPinsLow(uint8_t mask)
{
    _pPort->OUTCLR = mask;
}

uint8_t Port::GetPins()
{
    return _pPort->IN;
}

void Port::InterruptLevel(uint8_t num, uint8_t lvl)
{
    if (num == 0) {
        _pPort->INTCTRL &= ~(0x3);
        _pPort->INTCTRL |= (lvl & 0x3);
    } else {
        _pPort->INTCTRL &= ~(0xC);
        _pPort->INTCTRL |= (lvl & 0x3) << 2;
    }
}

void Port::InterruptMask(uint8_t num, uint8_t mask)
{
    if (num == 0) {
        _pPort->INT0MASK = mask;
    } else {
        _pPort->INT1MASK = mask;
    }
}

void Port::PinControl(uint8_t mask, 
        bool bSlewLimit,
        bool bInverted,
        PORT_OPC_t OutputConfig,
        PORT_ISC_t InputSense
        )
{
    //! The MPCMASK is a neat feature. I set each of the bits
    //! of the mask high, then configure any of the PINxCTRL
    //! registers, and only the pins specified in the mask get
    //! configured. Also, they all get the same config, so it's
    //! faster. It does not matter if I am actually configuring
    //! pin 0 or not, even though I specify PN0CTRL.
    PORTCFG.MPCMASK = mask;
    _pPort->PIN0CTRL = 
        (bSlewLimit ? 0x80 : 0x0) |
        (bInverted ? 0x40 : 0x0)  |
        OutputConfig | 
        InputSense
        ;
}


