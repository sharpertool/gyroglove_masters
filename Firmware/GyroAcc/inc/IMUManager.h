
#ifndef IMUManager_h
#define IMUManager_h

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include "TimerCntr.h"
#include "HardwareSerial.h"
#include "I2C_Master.h"
#include "IMU.h"

class IMUManager : public TimerNotify
{
    
    typedef enum StateType {
        sIdle, 
        sDataWait,
        sDataReady,
        sDataTimeout
    } StateType;
    
    IMUBase*        _pIMU[4];
    PORT_t*         _pBlueLed;
    uint8_t         _LedPin;
    bool            _bLedState;
    uint8_t         _LedCounter;
    uint8_t         _nIMUs;
    StateType       _State;
    uint16_t        _nStreamWDCounter;
    uint8_t         _packetId;
    
    // Keep a packet big enough for all data.
    uint8_t         _dataPacket[6*18+2];
    
    TimerCntr*      _pTimer;
    HardwareSerial* _pSerial;
    DebugPort*      _pDBGPort;
    uint16_t        _sampleRate;
    unsigned long   _maxMillisPerPacket;
    
    unsigned long   _lastSendMillis;
    
public:

    IMUManager(HardwareSerial* pSerial);
    
    int AddIMU(IMUBase* pIMU);
    void SetDebugPort(DebugPort* pPort);
    
    inline void SetState(StateType s)
    {
        _State = s;
        if (_pDBGPort) _pDBGPort->SetState((uint8_t)_State);
    }

    void Reset();
    int Loop();
    void Run();
    int StreamStart(bool bUseGyro = false);
    void SampleRate(uint16_t rate);
    void StreamWatchdog();
    
    int Setup();
    int Start();
    void Stop();
    void IMUUseGyro(bool bEn);
    void ForceStartStop();
    bool DataReady();
    void SendPacket(bool bTimeout);
    void DiscardData();
    
    // Timer Notification
    void SetTimer(TimerCntr* pTimer);
    void SetTimerPeriod();
    virtual void err(uint8_t id);
    virtual void ovf(uint8_t id);
    virtual void ccx(uint8_t id, uint8_t idx);
    
    // Diag stuff
    void CheckIDs(HardwareSerial* pSerial,int idx = -1);
    void ResetDevices();
    
    void SetBlueLed(PORT_t* port, uint8_t pin);
    
    void LedOff();
    void LedOn();
    void ToggleLed();
    void PacketLedIndicator();
    void ShowLedStart();
    void ShowLedStop();
    
    inline void ResetDataReadyTO()
    {
        _lastSendMillis = millis();
    }
  
    inline bool DataReadyTimeout()
    {
        if ((millis() - _lastSendMillis) > _maxMillisPerPacket) {
            return false;
        }
        return false;
    }

};

#endif

