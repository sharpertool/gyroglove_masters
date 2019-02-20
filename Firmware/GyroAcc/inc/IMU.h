
#ifndef IMU_h
#define IMU_h

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include "I2C_Master.h"
#include "HardwareSerial.h"
#include "IMUPacketFifo.h"
#include "TimerCntr.h"
#include "TimerCore.h"
#include "DebugPort.h"

class IMUBase
{
public:
    virtual void Reset() = 0;
    virtual void SampleRate(uint16_t) = 0;
    
    virtual int Setup() = 0;
    virtual int Start() = 0;
    virtual int Stop() = 0;
    virtual int ForceStartStop() = 0;
    virtual bool Busy() = 0;
    virtual void ResetTimer() = 0;
    virtual void UseGyro(bool bEnable) = 0;
    virtual void NextIMU(IMUBase*) = 0;
    virtual int BeginRead() = 0;
    
    virtual bool DataReady() = 0;
    virtual uint8_t* GetPacketData(uint8_t* pPacket) = 0;       
    
    // Diags
    virtual void CheckIDs(HardwareSerial* pSerial) = 0;
    virtual void ResetDevices() = 0;
};

class IMU : public IMUBase, public I2CNotify, public TimerNotify
{
    typedef enum StateType {
        sIdle           = 0,
        sConfigure      = 1,
        sConfigured     = 2,
        sWait           = 5,       
        sReadGyro1      = 8,
        sReadAcc1       = 9,
        sReadGyro2      = 10,
        sReadAcc2       = 11,
        sErrRecover     = 12
    } StateType;
    
    typedef enum PosType {
        pStart          = 0,
        pRun            = 1,
        pWriteDn        = 2,
        pReadDn         = 3,
        pNack           = 4,
        pBusErr         = 5,
        pArbLost        = 6,
        pSetup          = 7
    } PosType;
    
    typedef enum FailType {
        fNone           = 0,
        fNack           = 1,
        fBusErr         = 2,
        fArbLost        = 3
    } FailType;
    
    typedef struct regWrite {
        uint8_t     ID;
        uint8_t     Addr;
        uint8_t     Data;
    } RegWriteType;
    
    typedef enum  {
        ptTimer,
        ptI2CWrite,
        ptI2CRead,
        ptI2CNack
    } ProcessType;
    
    StateType       _State;
    StateType       _previousState; // Used for error recovery.
    FailType        _failType;
    
    I2C_Master*     _pMas;
    bool            _bDualChan;
    uint8_t         _numChans;
    bool            _configOkay[2];
    uint8_t         _gID[2];
    uint8_t         _aID[2];
    uint8_t         _DLPF;
    uint8_t         _FullScale;
    uint8_t         _ClkSel;
    uint16_t        _Rate;
    
    bool            _bUseGyro;

    //! This buffer is used to store data until we push it into the fifo
    uint8_t         _dataBuffer[2][20];
    bool            _bDataReady[2];
    
    TimerCntr*      _pTimer;
    bool            _bRun;
    uint16_t        _failCount; //! used for recover logic
    uint8_t         _nackCount;
    bool            _bFailDetected;
    unsigned int    _busyWaitTime;
    DebugPort*      _pDBGPort;
    DebugPort*      _pDBGPort2;
    IMUBase*        _pNextIMU;

public:
    IMU(I2C_Master* pMas);
    IMU(I2C_Master* pMas, uint8_t gID, uint8_t aID);
    IMU(I2C_Master* pMas, uint8_t gID, uint8_t aID,
        uint8_t gID2, uint8_t aID2
        );
    
    
    void QueryChannels();
    void SetDebugPort(DebugPort* pPort);
    void SetDebugPort2(DebugPort* pPort);
    
    virtual void Reset();
    virtual void SampleRate(uint16_t);
    
    virtual int Setup();
    virtual int Start();
    virtual int Stop();
    virtual int ForceStartStop();
    virtual bool Busy();
    virtual void ResetTimer();
    virtual void UseGyro(bool bEnable) { _bUseGyro = bEnable; };
    virtual void NextIMU(IMUBase* pNext);
    virtual int BeginRead();
    
    virtual bool DataReady();
    virtual uint8_t* GetPacketData(uint8_t*);

    // Diags
    virtual void CheckIDs(HardwareSerial* pSerial);
    virtual void ResetDevices();
    
    // Timer Notification
    void SetTimer(TimerCntr* pTimer);
    void SetTimerPeriod();
    virtual void err(uint8_t id);
    virtual void ovf(uint8_t id);
    virtual void ccx(uint8_t id, uint8_t idx);
    
    // I2C
    virtual void I2CWriteDone();
    virtual void I2CReadDone();
    virtual void I2CBusError();
    virtual void I2CArbLost();
    virtual void I2CNack();
    
    void FailRecovery();
    
protected:
    void Run();
    int StartTransaction();
    void ProcessTransaction();
    int Configure(uint8_t idx);
    int Wr(uint8_t ID, uint8_t addr, uint8_t data);
    int Rd(uint8_t ID, uint8_t addr, uint8_t cnt, uint8_t* pData);
    int WrAsync(uint8_t ID, uint8_t addr, uint8_t data);
    int RdAsync(uint8_t ID, uint8_t addr, uint8_t cnt);
    void ReadWord(uint16_t *pData);
    void StoreGyroData(uint8_t idx);
    void StoreAccData(uint8_t idx);
    void PushData(uint8_t idx);
    
    inline void SetState(StateType s)
    {
        _previousState = _State;
        _State = s;
        if (_pDBGPort) _pDBGPort->SetState((uint8_t)_State);
    }
    
    inline void MarkPos(PosType p)
    {
        if (_pDBGPort2) _pDBGPort2->SetState((uint8_t) p);
    }
    
    inline void ResetBusyTime()
    {
        _busyWaitTime = millis();
    }
    
    inline bool BusyTimeout()
    {
        return ((millis() - _busyWaitTime) > 2);
    }
    
    inline void ResetFailStats()
    {
        _bFailDetected      = false;
        _nackCount          = 0;
        _failCount          = 0;
        _failType           = fNone;
    }
        
};


#endif

