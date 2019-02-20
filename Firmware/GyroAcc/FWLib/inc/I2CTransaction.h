#ifndef I2CTransaction_h
#define I2CTransaction_h

#include <stdlib.h>
#include <inttypes.h>

class I2CTransaction
{
public:
    // This needs a state machine to keep track
    // of the state within a transation between interrups.
    typedef enum {
        sIdle,
        sSendID,
        sSendAddr,
        sWrite,
        sRead
    } StateType;
    
    typedef enum {
        tWrite,
        tRead,
        tIDScan
    } TransactionType;
    
    // These variables will track the current transation.
    StateType       _state;
    TransactionType _type;
    uint8_t*        _pBytes;
    uint8_t         _nByteCount;
    uint8_t         _nCurrPos;
    uint8_t         _nCurrID;
    uint8_t         _nCurrAddr;
    
    I2CTransaction(
        uint8_t     ID
        ) 
    {
        _state = sIdle;
        _type = tIDScan;
        _nCurrID = ID;
        _nCurrAddr = 0;
        _pBytes = 0;
        _nCurrPos =0;
        _nByteCount = 0;
    }
    
    I2CTransaction(
        uint8_t     ID,
        uint8_t     Addr,
        uint8_t     nBytes
        ) 
    {
        _state = sIdle;
        _type = tRead;
        _nCurrID = ID;
        _nCurrAddr = Addr;
        _pBytes = (uint8_t*)malloc(nBytes);
        _nCurrPos =0;
        _nByteCount = nBytes;
    }
    
    I2CTransaction(
        uint8_t     ID,
        uint8_t     Addr,
        uint8_t*    pData,
        uint8_t     nBytes
        ) 
    {
        _state = sIdle;
        _type = tWrite;
        _nCurrID = ID;
        _nCurrAddr = Addr;
        _pBytes = (uint8_t*)malloc(nBytes);
        memcpy(_pBytes,pData,nBytes);
        _nCurrPos = 0;
        _nByteCount = nBytes;
    }
    
    ~I2CTransaction() { free(_pBytes); }
    
    StateType state() { return _state; } 
        
};

#endif

