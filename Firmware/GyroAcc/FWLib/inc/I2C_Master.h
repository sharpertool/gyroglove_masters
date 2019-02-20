
#ifndef I2C_Master_h
#define I2C_Master_h

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>

#include "I2CTransaction.h"

// Derive a class from this class
// in order to give the class the ability
// to be notified when a write or read is done.
class I2CNotify
{
public:
    virtual void I2CWriteDone() = 0;
    virtual void I2CReadDone() = 0;
    virtual void I2CBusError() = 0;
    virtual void I2CArbLost() = 0;
    virtual void I2CNack() = 0;
};


class I2C_Master
{
public:
    typedef enum {
        sIdle,
        sBusy,
        sError,
        sArb,
        sIDScan,
        sIDCheck
    } DriverState;

    typedef enum {
        rOk,
        rFail,
        rArbLost,
        rBussErr,
        rNack,
        rBufferOverrun,
        rUnknown,
        rTimeout
    } DriverResult;
    
private:
    TWI_t*          _twi;
    PORT_t*         _twiPort;
    bool            _bEnabled;
    DriverState     _State;
    DriverResult    _Result;
    void*           _pReserved;
    I2CNotify*      _pNotifyClient;
    
    // Transaction Data
    uint8_t         _DeviceID;
    uint8_t         _nBytesWritten;
    uint8_t         _nWriteBytes;
    uint8_t         _nReadBytes;
    uint8_t         _nBytesRead;
    
    uint8_t*        _WriteData;
    uint8_t         _wrBufferLen;
    uint8_t*        _ReadData;
    uint8_t         _rdBufferLen;
    
    //I2CTransaction* _pTransaction;
    
    // For ID Scanning
    uint8_t         _idScanCurrent;
    uint8_t         _IDList[128];
    bool            _ScanComplete;

public:
    
    typedef enum ErrorType {
        eNone           = 0,
        eDisabled       = -1,
        eBusy           = -2,
        eNack           = -3,
        eArbLost        = -4,
        eBusErr         = -5,
        eTimeout        = -6,
        eSDAStuck       = -7,
        eSCLStuck       = -8,
        eUnknown        = -9
    } ErrorType;
    
    I2C_Master(TWI_t*  twi);
    ~I2C_Master();
    
    //
    void begin(uint32_t freq);
    void end();
    
    // Perform a write. Return status indicates result
    // < 0 indicates an error: -1 == NACK -1 = Some other error???
    // > 0 indicates # of bytes written with valid ACK.
    ErrorType Write(uint8_t ID, uint8_t* Data, uint8_t nBytes);
    ErrorType WriteSync(uint8_t ID, uint8_t* Data, uint8_t nBytes);
    
    // Perform a read. Return status indicates result.
    // maxcnt indidcates how many bytes to attempt to read. Will read until
    // a NACK occurs or maxcnt bytes are read.
    // RETC < 0: -1 -> NACK of ID. 
    // RETC > 0: number of bytes read (before NACK from Slave
    ErrorType Read(uint8_t ID, uint8_t nBytes);
    ErrorType ReadSync(uint8_t ID, uint8_t nBytes);
    
    ErrorType WriteRead(uint8_t ID,
                  uint8_t* wrData,
                  uint8_t nWriteBytes, 
                  uint8_t nReadBytes);
    ErrorType WriteReadSync(uint8_t ID,
                  uint8_t* wrData,
                  uint8_t nWriteBytes, 
                  uint8_t nReadBytes);
    
    
    void master_int();
    void slave_int();
    
    void WriteHandler();
    void ReadHandler();
    void ArbHandler();
    void ErrorHandler();
    
    void MasterFinished();
    int testack(uint8_t ID);
    void dumpregs();
    
    I2C_Master::DriverResult Result();
    I2C_Master::DriverState State();
    uint8_t ReadData(uint8_t* pData, uint8_t maxcnt);
    uint8_t ReadData(uint8_t index);
    uint8_t nReadBytes();
    
    ErrorType CheckID(uint8_t ID);
    void Stop();      // Use I2C Master for stop.
    ErrorType ForceStartStop();
    ErrorType WigglePin(uint8_t cnt, uint8_t pinSel, uint8_t otherState);
    void CleanRegs();
    
    void loop(); // Called periodically.
    
    bool busy(); // In the process of transacting...
    void* isReserved();
    bool Reserve(void*);
    void NotifyMe(I2CNotify* pMe);
    
    inline bool IsIdle()
    {
        return (_twi->MASTER.STATUS & TWI_MASTER_BUSSTATE_gm) 
            == TWI_MASTER_BUSSTATE_IDLE_gc;
    }

protected:
    uint8_t busState();
    void showstate();
    

};


#endif

