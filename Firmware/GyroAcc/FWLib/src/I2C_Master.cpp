#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "HardwareSerial.h"

#include "I2C_Master.h"

extern HardwareSerial* pdbgserial;

// Generate all of the ISR handlers.. hook them up to a class if/when a class
// is instantiated for a particular USART.
#define I2C_ISR_DEF(periph) \
static I2C_Master*  periph##_master_cp = 0;\
ISR(periph##_TWIM_vect) {\
    if (periph##_master_cp) periph##_master_cp->master_int();\
} \
ISR(periph##_TWIS_vect) {\
    if (periph##_master_cp) periph##_master_cp->slave_int();\
}

I2C_ISR_DEF(TWIC);
I2C_ISR_DEF(TWIE);
#if defined(__AVR_ATxmega128A1__)
I2C_ISR_DEF(TWID);
I2C_ISR_DEF(TWIF);
#endif

static void SetTWIPointer(TWI_t* ptype, I2C_Master* pm)
{
    if(ptype == &TWIC) {
        TWIC_master_cp = pm;
#if defined (__AVR_ATxmega128A1__)
    } else if (ptype == &TWID) {
        TWID_master_cp = pm;
    } else if (ptype ==  &TWIF) {
        TWIF_master_cp = pm;
#endif
    } else if (ptype ==  &TWIE) {
        TWIE_master_cp = pm;
    }
}

I2C_Master::I2C_Master(TWI_t* twi)
{
    _twi        = twi;
    _bEnabled   = false;
    _State      = sIdle;
    _Result     = rOk;

    _WriteData      = 0;
    _ReadData       = 0;
    _wrBufferLen    = 0;
    _rdBufferLen    = 0;
    _DeviceID       = 0;
    _nBytesWritten  = 0;
    _nWriteBytes    = 0;
    _nReadBytes     = 0;
    _nBytesRead     = 0;
    _pReserved      = 0;
    _pNotifyClient  = 0;

    _ScanComplete = 0;
    
    //! Set the port so we can force some stop conditions.
#if defined(__AVR_ATxmega128A1__)
    if (twi == &TWIC) {
        _twiPort = &PORTC;
    } else if (twi == &TWID) {
        _twiPort = &PORTD;
    } else if (twi == &TWIE) {
        _twiPort = &PORTE;
    } else {
        _twiPort = &PORTF;
    }
#else
    if (twi == &TWIC) {
        _twiPort = &PORTC;
    } else {
        _twiPort = &PORTE;
    }
#endif
    
    PORTCFG.MPCMASK = 0x3;
    _twiPort->PIN0CTRL =  
            PORT_OPC_WIREDANDPULL_gc 
          | PORT_ISC_BOTHEDGES_gc;
    _twiPort->DIRSET = 0x3;
    _twiPort->OUTSET = 0x3; // Set both high, let float.

    SetTWIPointer(_twi,this);
    if (pdbgserial) pdbgserial->println("I2C Master Constructor.");
}

I2C_Master::~I2C_Master()
{
    end();

    SetTWIPointer(_twi,(I2C_Master*)0);
}

void I2C_Master::NotifyMe(I2CNotify* pMe)
{
    _pNotifyClient = pMe;
}

//! Clean up the I2C Master registers without an end/begin.
void I2C_Master::CleanRegs()
{
    _twi->MASTER.STATUS = 
        TWI_MASTER_RIF_bm |
        TWI_MASTER_WIF_bm |
        TWI_MASTER_ARBLOST_bm |
        TWI_MASTER_BUSERR_bm |
        TWI_MASTER_BUSSTATE_IDLE_gc;
}

void I2C_Master::begin(uint32_t freq)
{
    // Things to do:
    // Check the bus state. If the state is Unknown, then issue a stop to get the
    // state to Idle.

    _twi->CTRL = 0; // Set to normal TWI, turn off SDA Hold

    // Clear any current interrupts
    _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;

    // Baud calculations..
    uint8_t baud;
    //float float_freq = freq;
    baud = (F_CPU/(2*freq)) - 5;
    _twi->MASTER.BAUD = baud; // Set Baud Rate
    
    // I had this set to zero timeout.. which I think was bad. I would occasionally
    // get ARB Lost and this would never reset. Maybe this will help.
    _twi->MASTER.CTRLB = TWI_MASTER_TIMEOUT_200US_gc;
    //_twi->MASTER.CTRLB = 0;
    //_twi->MASTER>CTRLC used during operation.

    // Set the interrupt level and enable the master.
    _twi->MASTER.CTRLA =
          TWI_MASTER_INTLVL_LO_gc
        | TWI_MASTER_ENABLE_bm
        | TWI_MASTER_RIEN_bm
        | TWI_MASTER_WIEN_bm;

    // Force bus state to Idle.
    _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;

    _bEnabled   = true;
    _State      = sIdle;
    _Result     = rOk;
    if (pdbgserial) pdbgserial->println("I2C Master begin called.");
}

void I2C_Master::end()
{
    _twi->MASTER.CTRLA = 0; // Disable everything.
    _bEnabled = false;
    if (pdbgserial) pdbgserial->println("I2C Master end called.");
}

void I2C_Master::loop()
{
    if (_State == sError) {
        _State = sIdle;
        if (pdbgserial) pdbgserial->println("Reset after error.");
    }
}

int I2C_Master::testack(uint8_t ID)
{
    char buffer[128];

    sprintf(buffer,"Testing TWI:ID = %d",ID);
    if (pdbgserial) pdbgserial->println(buffer);

    _twi->MASTER.CTRLA &= ~TWI_MASTER_ENABLE_bm;
    _twi->MASTER.BAUD = 35;
    _twi->MASTER.CTRLA |= TWI_MASTER_ENABLE_bm;

    // Clear any current interrupts
    _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
    showstate();

    dumpregs();

    uint8_t addr = ID & ~0x1; // Write
    _twi->MASTER.ADDR = addr;

    uint8_t to = 5;
    while (--to && _State != sIdle) {
        _delay_ms(10);
        showstate();
        dumpregs();
    }
    if (to) {
        showstate();
        dumpregs();
        _twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        showstate();
        dumpregs();
    } else {
        if (pdbgserial) pdbgserial->println("Timeout..");
    }
    showstate();

	return 0;
}

uint8_t I2C_Master::busState()
{
    return (_twi->MASTER.STATUS & TWI_MASTER_BUSSTATE_gm);
}

I2C_Master::ErrorType I2C_Master::CheckID(uint8_t ID)
{
    char buffer[128];
    sprintf(buffer,"Check id 0x%x",ID);
    if (pdbgserial) pdbgserial->println(buffer);

    //showstate();

    ErrorType retc;
    if ((retc = Write(ID,0,0)) < 0) {
        // Error starting the write
        // Values -1, -2
        return retc;
    }

    //showstate();

    uint8_t timeout = 100;
    while (--timeout && _State != sIdle) {
        _delay_us(100); // Busy Wait..
    }

    //showstate();

    if (timeout == 0) {
        if (pdbgserial) pdbgserial->println("Timeout waiting for completion of command.");
        _Result = rTimeout;
        _State  = sIdle;
        _twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        return  eTimeout;
    } else {
        if (_Result == rOk) {
            return eNone;
        } else if (_Result == rNack) {
            return eNack;
        }
    }
    return eUnknown;
}

// Copy the bytes to be sent into the send buffer, then
// start sending a byte at a time. The interrupt will
// be used to make this call asynchronous.
I2C_Master::ErrorType I2C_Master::Write(uint8_t ID, uint8_t* pData, uint8_t nWrBytes)
{
    return WriteRead(ID, pData, nWrBytes, 0);
}

I2C_Master::ErrorType I2C_Master::WriteSync(uint8_t ID, uint8_t* pData, uint8_t nWrBytes)
{
    return WriteReadSync(ID, pData, nWrBytes, 0);
}

I2C_Master::ErrorType I2C_Master::Read(uint8_t ID, uint8_t nRdBytes)
{
    return WriteRead(ID, 0, 0, nRdBytes);
}

I2C_Master::ErrorType I2C_Master::ReadSync(uint8_t ID, uint8_t nRdBytes)
{
    return WriteReadSync(ID, 0, 0, nRdBytes);
}

I2C_Master::ErrorType I2C_Master::WriteReadSync(  uint8_t ID,
                            uint8_t* wrData,
                            uint8_t nWriteBytes,
                            uint8_t nReadBytes)
{
    //if (pdbgserial) pdbgserial->println("Start=========== Sending Async Version.");
    //sprintf(buffer,"nWriteBytes:%d nReadBytes:%d\n",nWriteBytes, nReadBytes);
    //if (pdbgserial) pdbgserial->print(buffer);
    ErrorType retc = WriteRead(ID, wrData, nWriteBytes, nReadBytes);

    if (retc != eNone) return retc;

    uint8_t timeout = 100;
    while (--timeout && _State != sIdle) {
        _delay_us(100); // Busy Wait..
    }

    // If a timeout occured, close the transaction and return an error.
    if (timeout == 0) {
        if (pdbgserial) pdbgserial->println("Wait Timeout.");
        _Result = rTimeout;
        _State  = sIdle;
        _twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        return  eTimeout;
    }

    // The transaction completed, but was it right?
    if (_Result != rOk) {
        if (_Result == rNack) {
            retc = eNack;
        } else {
            retc = eUnknown;
        }
    }
    
    return retc;
}

I2C_Master::ErrorType I2C_Master::WriteRead(  uint8_t ID,
                            uint8_t* wrData,
                            uint8_t nWriteBytes,
                            uint8_t nReadBytes)
{
    if (_bEnabled == false) return eDisabled;
    if (busy()) return eBusy;

    if (nWriteBytes > _wrBufferLen) {
        free(_WriteData);
        _WriteData = (uint8_t*)malloc(nWriteBytes+1);
        _wrBufferLen = nWriteBytes;
        //if (pdbgserial) pdbgserial->println("Allocated write buffer.");
    }
    memcpy(_WriteData,wrData,nWriteBytes);
    _nBytesWritten  = 0;
    _nWriteBytes    = nWriteBytes;

    if (nReadBytes > _rdBufferLen) {
        free(_ReadData);
        _ReadData = (uint8_t*)malloc(nReadBytes+1);
        _rdBufferLen = nReadBytes;
        //if (pdbgserial) pdbgserial->println("Allocated read buffer.");
    }
    _nBytesRead     = 0;
    _nReadBytes     = nReadBytes;

    _Result     = rUnknown;

    _DeviceID   = ID;

    if (_nWriteBytes > 0) {
        // Start the write transaction..
        uint8_t addr = _DeviceID & ~0x1; // Write
        _twi->MASTER.ADDR = addr;
    } else if (_nReadBytes) {
        uint8_t addr = _DeviceID | 0x1; // Read
        _twi->MASTER.ADDR = addr;
    } else {
        // Nothing to write, but we are pinging the ID
        uint8_t addr = _DeviceID & ~0x1; // Write
        _twi->MASTER.ADDR = addr;
    }
    _State      = sBusy;

    return eNone;
}

uint8_t I2C_Master::ReadData(uint8_t idx)
{
    if (_State == sIdle && _Result == rOk && idx < _nBytesRead) {
        return _ReadData[idx];
    }
    return 0;
}

uint8_t I2C_Master::ReadData(uint8_t* pData, uint8_t maxcnt)
{
    if (_State == sIdle && _Result == rOk) {
        uint8_t nCopy = maxcnt <= _nBytesRead ?
            maxcnt : _nBytesRead;
        memcpy(pData,_ReadData,nCopy);
        return nCopy;
    }
    return 0;
}

uint8_t I2C_Master::nReadBytes() 
{
    return _nBytesRead; 
}

bool I2C_Master::busy()
{
    if (_State != sIdle) return true;

    return false;
}

void* I2C_Master::isReserved()
{
    return _pReserved;
}


bool I2C_Master::Reserve(void* pThis)
{
    if (!_pReserved) {
        _pReserved = pThis;
        return true;
    } else {
        if (_pReserved == pThis) {
            return true;
        }
    }
    return false;
}

I2C_Master::DriverState I2C_Master::State()
{
    return _State;
}

I2C_Master::DriverResult I2C_Master::Result()
{
    return _Result;
}

void I2C_Master::Stop()
{
    _twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

I2C_Master::ErrorType I2C_Master::ForceStartStop()
{
    // Wait for master to be idle.
    uint16_t waitTimeout = 1000;
    while (waitTimeout && !IsIdle()) {
        _delay_us(1);
        --waitTimeout;
    }
    
    if (!waitTimeout) {
        return eTimeout; // Timeout.
    }
    
    uint8_t oldState = _twi->MASTER.CTRLA; 
    _delay_us(2);
    _twi->MASTER.CTRLA = 0; // Turn off interrupts and disable master.
    
    _twiPort->DIRSET = 0x3;
    _twiPort->OUTSET = 0x3; // Set SCL and SDA High
    _delay_us(5);
    
    if ((_twiPort->IN & 0x1) == 0) {
        // SDA Line Hung low!
        _twi->MASTER.CTRLA = oldState;
        _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
        return eSDAStuck;
    }
    
    if ((_twiPort->IN & 0x2) == 0) {
        _twi->MASTER.CTRLA = oldState;
        _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
        return eSCLStuck;
    }
    
    _twiPort->OUTCLR = 0x1; // Set SDA Low 
    _delay_us(2);
    _twiPort->OUTCLR = 0x2; // Set SCL Low
    
    for (int x=0;x<9;x++) {
        _delay_us(3);
        _twiPort->OUTSET = 0x2; // SCL Hi
        _delay_us(3);
        _twiPort->OUTCLR = 0x2; // SCL Lo
    }
    
    _delay_us(3);
    _twiPort->OUTSET = 0x2; // SCL Hi
    _delay_us(3);
    _twiPort->OUTSET = 0x1; // Set SDA High - Stop
    _delay_us(2);
    
    // Restore the Master to on.
    _twi->MASTER.CTRLA = oldState;
    _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
    return eNone;
}

I2C_Master::ErrorType I2C_Master::WigglePin(uint8_t cnt, uint8_t pinSel, uint8_t otherState)
{
    // Wait for master to be idle.
    uint16_t waitTimeout = 1000;
    while (waitTimeout && !IsIdle()) {
        _delay_us(1);
        --waitTimeout;
    }
    
    if (!waitTimeout) {
        return eTimeout; // Timeout.
    }
    
    uint8_t oldState = _twi->MASTER.CTRLA; 
    _twi->MASTER.CTRLA = 0; // Turn off interrupts and disable master.
    
    _twiPort->OUTSET = 0x3; // Set SCL and SDA High
    _delay_us(5);
    
    if ((_twiPort->IN & 0x1) == 0) {
        // SDA Line Hung low!
        _twi->MASTER.CTRLA = oldState;
        _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
        return eSDAStuck;
    }
    
    if ((_twiPort->IN & 0x2) == 0) {
        _twi->MASTER.CTRLA = oldState;
        _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
        return eSCLStuck;
    }
    
    // Mask for the pin to set.
    uint8_t pinA = (pinSel == 1) ? 0x2 : 0x1;
    uint8_t pinB = (pinSel == 1) ? 0x1 : 0x2;
    
    if (otherState == 0) {
        // Force other pin low,
        _twiPort->OUTCLR = pinB;
        _delay_us(2);
    }
    
    for (int x=0;x<cnt;x++) {
        _delay_us(3);
        _twiPort->OUTSET = pinA;
        _delay_us(3);
        _twiPort->OUTCLR = pinA;
    }
    
    _delay_us(3);
    _twiPort->OUTSET = pinA;
    _delay_us(3);
    _twiPort->OUTSET = pinB;
    _delay_us(2);
    
    // Restore the Master to on.
    _twi->MASTER.CTRLA = oldState;
    _twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
    return eNone;
}

void I2C_Master::master_int()
{
    //if (pdbgserial) pdbgserial->println("Master Interrupt.");
    uint8_t currentStatus = _twi->MASTER.STATUS;

    if (currentStatus & TWI_MASTER_BUSERR_bm) {
        // I have found that on some occasions the 
        // slave releases the NACK and the SDA line goes high
        // too quickly, before SCL drops. I do not know why this
        // is allowed to occur.. but I'm going to try and keep going
        // if possible.
        if (currentStatus & TWI_MASTER_WIF_bm) {
            // Clear the Bus error but retain the Bus state and
            // RXAck bits
            _twi->MASTER.STATUS = 
                TWI_MASTER_BUSERR_bm 
                | (currentStatus & (TWI_MASTER_RXACK_bm | 0x3));
            WriteHandler();
        } else {
            ErrorHandler();
        }
    }

    if (currentStatus & TWI_MASTER_ARBLOST_bm) {
        ArbHandler();
    }

    if (currentStatus & TWI_MASTER_WIF_bm) {
        WriteHandler();
    }

    if (currentStatus & TWI_MASTER_RIF_bm) {
        ReadHandler();
    }
}

/**********************************
 If the slave did not acknowledge, then terminate
 the transaction. If the slave did acknowledge, then
 if there are more byts to write, send the next byte,
 otherwise, if there are bytes to read, start the read
 process with another ID write.
 **********************************/
void I2C_Master::WriteHandler()
{
    if (_twi->MASTER.STATUS & TWI_MASTER_RXACK_bm) {
        _twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        _Result     = rNack;
        _State      = sIdle;
        // This might be the end of the write, and no read is required
        // in which case the Slave may Nack. In this case the Nack is
        // okay.
        if (_pNotifyClient) {
            _pNotifyClient->I2CNack();
        }
    } else if (_nBytesWritten < _nWriteBytes) {
        // More bytes to write
        uint8_t data = _WriteData[_nBytesWritten];
        ++_nBytesWritten;
        _twi->MASTER.DATA = data;
        //if (pdbgserial) pdbgserial->println("I_WIF - Write");
    } else if (_nBytesRead < _nReadBytes) {
        // This will be a transition from a write to a
        // read, so we re-send the ID with the READ bit
        // set.
        _twi->MASTER.ADDR = _DeviceID | 0x1;
        //if (pdbgserial) pdbgserial->println("I_WIF - Read");
    } else {
        // The transaction is done.. close it.
        _twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        _Result = rOk;
        _State = sIdle;
        //if (pdbgserial) pdbgserial->println("I_WIF - Done");
        if (_pNotifyClient) {
            _pNotifyClient->I2CWriteDone();
        }
    }

    // Not sure if I need to clear this or not..
    // No - all of the above cases will clear it.
    //_twi->MASTER.STATUS |= TWI_MASTER_WIF_bm;
}

void I2C_Master::ReadHandler()
{
    /*
    This member function is called by the static function
    associated with a particular I2C hardware resource.
    The connection is set in the constructore for I2C Master.
    Anyway, this allows each class instance to handle it's
    own interupts.
    */
    if (_nBytesRead < _rdBufferLen) {
        uint8_t data = _twi->MASTER.DATA;
        _ReadData[_nBytesRead] = data;
        ++_nBytesRead;

        if (_nBytesRead < _nReadBytes) {
            _twi->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
        } else {
            // No more bytes to read..  Nack and stop
            _twi->MASTER.CTRLC = TWI_MASTER_ACKACT_bm |
                                 TWI_MASTER_CMD_STOP_gc;
    
            _Result     = rOk;
            _State      = sIdle;
            if (_pNotifyClient) {
                _pNotifyClient->I2CReadDone();
            }
        }
    } else {
        // Buffer Overrun. Stop and quit with the bytes
        _twi->MASTER.CTRLC = TWI_MASTER_ACKACT_bm |
                            TWI_MASTER_CMD_STOP_gc;
        _State  = sIdle;
        _Result = rBufferOverrun;

        if (_pNotifyClient) {
            _pNotifyClient->I2CReadDone();
        }
    }

}

void I2C_Master::ArbHandler()
{
    _twi->MASTER.STATUS |= TWI_MASTER_ARBLOST_bm;
    _State  = sIdle;
    _Result = rArbLost;
    if (_pNotifyClient) {
        _pNotifyClient->I2CArbLost();
    }
    return;
}

void I2C_Master::ErrorHandler()
{
    // Clear the error flag.
    _twi->MASTER.STATUS |= TWI_MASTER_BUSERR_bm;
    
    // Stop with a NACK. Not sure ifa NACK is needed here, but that
    // is the best solution if it happens to be needed.
    _twi->MASTER.CTRLC = TWI_MASTER_ACKACT_bm |
                                 TWI_MASTER_CMD_STOP_gc;
    _State  = sIdle;
    _Result = rBussErr;
    if (_pNotifyClient) {
        _pNotifyClient->I2CBusError();
    }
}

void I2C_Master::slave_int()
{
    // Not Implemented..
}

void I2C_Master::showstate()
{
    switch(busState()) {
        case TWI_MASTER_BUSSTATE_UNKNOWN_gc:
            if (pdbgserial) pdbgserial->println("BusState -> Unknown.");
            break;
        case TWI_MASTER_BUSSTATE_IDLE_gc:
            if (pdbgserial) pdbgserial->println("BusState -> Idle.");
            break;
        case TWI_MASTER_BUSSTATE_OWNER_gc:
            if (pdbgserial) pdbgserial->println("BusState -> Owner.");
            break;
        case TWI_MASTER_BUSSTATE_BUSY_gc:
            if (pdbgserial) pdbgserial->println("BusState -> Busy.");
            break;
    }
}

void I2C_Master::dumpregs()
{
    char    buffer[128];

    sprintf(buffer,"CTRLA:0x%x",_twi->MASTER.CTRLA);
    if (pdbgserial) pdbgserial->println(buffer);
    sprintf(buffer,"CTRLB:0x%x",_twi->MASTER.CTRLB);
    if (pdbgserial) pdbgserial->println(buffer);
    sprintf(buffer,"CTRLC:0x%x",_twi->MASTER.CTRLC);
    if (pdbgserial) pdbgserial->println(buffer);
    sprintf(buffer,"BAUD:0x%x",_twi->MASTER.BAUD);
    if (pdbgserial) pdbgserial->println(buffer);
    sprintf(buffer,"STATUS:0x%x",_twi->MASTER.STATUS);
    if (pdbgserial) pdbgserial->println(buffer);

    sprintf(buffer,"SLAVE.CTRLA:0x%x",_twi->SLAVE.CTRLA);
    if (pdbgserial) pdbgserial->println(buffer);
}




