#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "HardwareSerial.h"
#include "QuadDecoder.h"

extern HardwareSerial* pdbgserial;

QuadDecoder::QuadDecoder(
    // Port
    PORT_t      *port,
    uint8_t     portPin,
    bool        bUseIndex,
    bool        bInvertIO,
    
    // Event
    uint8_t         qEvMux,
    EVSYS_CHMUX_t   qPinInput,
    EVSYS_QDIRM_t   qIndexState,
    
    // Timer Counter
    TC0_t*          pTimer,
    TC_EVSEL_t      qEventChannel,
    
    // Hardare definition
    uint16_t         lineCount = 32
    )
{
    _port       = port;
    _pPin       = portPin;
    _binvIO     = bInvertIO;
    _bUseIndex  = bUseIndex;
    
    _evMux      = qEvMux;
    _evPinInput = qPinInput;
    _evIndexState   = qIndexState;
    
    _pTimer     = pTimer;
    _tcEventChannel = qEventChannel;
    
    _lineCount  = lineCount;
    _theCount   = 0;
    _bUsePort   = false;
}

bool QuadDecoder::Setup()
{
    pdbgserial->println("Setting up Port");
    if (!PortSetup()) {
        return false;
    }
        
    pdbgserial->println("Setting up Events");
    if (!EventSetup()) {
        return false;
    }
    
    pdbgserial->println("Setting up Counter");
    TC_DecSetup();
    
    pdbgserial->println("Setup Done.");
    return true;
}

void QuadDecoder::PinStatus(char& ph0, char& ph90)
{
    uint8_t in = _port->IN;
    ph0 = (in & (0x1 << _pPin)) ? '1' : '0';
    ph90 = (in & (0x2 << _pPin)) ? '1' : '0';
}

bool QuadDecoder::PortSetup()
{
    /* Make setup depending on if Index signal is used. */
    if(_bUseIndex){
        if(_pPin > 5){
            return false;
        }
        _port->DIRCLR = (0x07<<_pPin);

        /* Configure Index signal sensing. */
        PORTCFG.MPCMASK = (0x04<<_pPin);
        _port->PIN0CTRL = (_port->PIN0CTRL & ~PORT_ISC_gm) | PORT_ISC_BOTHEDGES_gc
                          | (_binvIO ? PORT_INVEN_bm : 0);


    }else{
        if(_pPin > 6){
            return false;
        }
        _port->DIRCLR = (0x03<<_pPin);
    }

    /* Set QDPH0 and QDPH1 sensing level. */
    PORTCFG.MPCMASK = (0x03<<_pPin);
    _port->PIN0CTRL = (_port->PIN0CTRL & ~PORT_ISC_gm) | PORT_ISC_LEVEL_gc
                      | (_binvIO ? PORT_INVEN_bm : 0);

    return true;
}

bool QuadDecoder::EventSetup()
{
    switch (_evMux){
        case 0:
            
        /* Configure event channel 0 for quadrature decoding of pins. */
        EVSYS.CH0MUX = _evPinInput;
        EVSYS.CH0CTRL = EVSYS_QDEN_bm | EVSYS_DIGFILT_2SAMPLES_gc;
        if(_bUseIndex){
            /*  Configure event channel 1 as index channel. Note
             *  that when enabling Index in channel n, the channel
             *  n+1 must be configured for the index signal.*/
            EVSYS.CH1MUX = _evPinInput + 2;
            EVSYS.CH1CTRL = EVSYS_DIGFILT_2SAMPLES_gc;
            EVSYS.CH0CTRL |= (uint8_t) _evIndexState | EVSYS_QDIEN_bm;

        }
        break;
        case 2:
        EVSYS.CH2MUX = _evPinInput;
        EVSYS.CH2CTRL = EVSYS_QDEN_bm | EVSYS_DIGFILT_2SAMPLES_gc;
        if(_bUseIndex){
            EVSYS.CH3MUX = _evPinInput + 2;
            EVSYS.CH3CTRL = EVSYS_DIGFILT_2SAMPLES_gc;
            EVSYS.CH2CTRL |= (uint8_t) _evIndexState | EVSYS_QDIEN_bm;
        }
        break;
        case 4:
        EVSYS.CH4MUX = _evPinInput;
        EVSYS.CH4CTRL = EVSYS_QDEN_bm | EVSYS_DIGFILT_2SAMPLES_gc;
        if(_bUseIndex){
            EVSYS.CH5MUX = _evPinInput + 2;
            EVSYS.CH5CTRL = EVSYS_DIGFILT_2SAMPLES_gc;
            EVSYS.CH4CTRL |= (uint8_t) _evIndexState | EVSYS_QDIEN_bm;
        }
        break;
        default:
        return false;
    }
    return true;
}

void QuadDecoder::TC_DecSetup()
{
    /* Configure TC as a quadrature counter. */
    _pTimer->CTRLD = (uint8_t) TC_EVACT_QDEC_gc | _tcEventChannel;
    _pTimer->PER = (_lineCount * 4) - 1;
    _pTimer->CTRLA = TC_CLKSEL_DIV1_gc;
    _pTimer->CTRLFSET |= TC0_DIR_bm;
}

void QuadDecoder::TC_FreqSetup()
{
    /* Configure channel 2 to input pin for freq calculation. */
    EVSYS.CH2MUX = _evPinInput;
    EVSYS.CH2CTRL = EVSYS_DIGFILT_4SAMPLES_gc;

    /* Configure TC to capture frequency. */
    _pTimer->CTRLD = (uint8_t) TC_EVACT_FRW_gc | _tcEventChannel;
    _pTimer->PER = 0xFFFF;
    _pTimer->CTRLB = TC0_CCAEN_bm;
    
    // ToDo: Not sure about this, review it..
    //_pTimer->CTRLA = clksel;
    //_pTimer->CTRLA = CLOCK_DIV_bm;
}

QuadDecoder::DirType QuadDecoder::Get_Direction()
{
    if (_pTimer->CTRLFSET & TC0_DIR_bm){
        return CW;
    }else{
        return CCW;
    }
}

uint16_t QuadDecoder::Count()
{
    if (_bUsePort) {
        return _theCount;
    }
    
    uint16_t val = _pTimer->CNT;
    //uint16_t val = (_pTimer->CNTH << 8 | _pTimer->CNTL);
    if (_bInverted) 
        return ~val;
    
    
    return val;
}

// This is for debug purposes.
void QuadDecoder::SetCount(uint16_t cnt)
{
    if (_bInverted) {
        cnt = ~cnt;
    }
    
    _pTimer->CNT = cnt;
    //_pTimer->CNTH = (cnt & 0xff00) >> 8;
    //_pTimer->CNTL = (cnt & 0xff);
    _theCount = cnt;
}

bool QuadDecoder::IsInverted()
{
    return _bInverted;
}

void QuadDecoder::Invert(bool bInv)
{
    _bInverted = bInv;
}

void QuadDecoder::PortISR0(uint8_t id)
{
    // Phase 0
    if (id == 0) {
        _theCount++;
        char buffer[100];
        sprintf(buffer,"ISR0 Ph0 %d\n",_theCount);
        pdbgserial->print(buffer);
    }
}

void QuadDecoder::PortISR1(uint8_t id)
{
    // Phase 90
    if (id == 0) {
        _theCount++;
        char buffer[100];
        sprintf(buffer,"ISR1 Ph90 %d\n",_theCount);
        pdbgserial->print(buffer);
    }
}

void QuadDecoder::SetPort(Port* pp, uint8_t pin0, uint8_t pin90)
{
    _pPort = pp;
    _pPort->SetPinsAsInput(pin0 | pin90);
    _bUsePort = true;
    _pPort->PinControl(
        pin0 | pin90,   // Pin Mask
        true,                       // Slew Rate Limited
        false,                      // Not Inverted
        PORT_OPC_TOTEM_gc,          // Use internal Pull-UP
        PORT_ISC_BOTHEDGES_gc);     // Monitor Both edges.
    _pPort->InterruptMask(0,pin0);
    _pPort->InterruptLevel(0,1);
    _pPort->InterruptMask(1,pin90);
    _pPort->InterruptLevel(1,1);
    _pPort->Notify(this,0);
        
    
    pdbgserial->print("Setup Port\n");

}



