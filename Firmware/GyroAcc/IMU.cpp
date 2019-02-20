#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "HardwareSerial.h"
#include "IMUPacketFifo.h"
#include "TimerCore.h"

#include "IMU.h"
#include "Mark.h"

extern HardwareSerial* pdbgserial;
static char buffer[128];

//! Constructor for an auto-query channel
//! Init fifos for dual channel
IMU::IMU(I2C_Master* pMas)
{
    _pNextIMU       = 0;
    _gID[0]         = 0;
    _aID[0]         = 0;
    _gID[1]         = 0;
    _aID[1]         = 0;
    _bDualChan      = false;
    _numChans       = 0;
    _pMas           = pMas;
    _DLPF           = 0x1;
    _FullScale      = 0x1;
    _ClkSel         = 0x1;
    _Rate           = 10;
    _State          = sIdle;
    _previousState  = sIdle;
    _failType       = fNone;
    _bRun           = false;
    _busyWaitTime   = 0;
    _bDataReady[0]  = false;
    _bDataReady[1]  = false;
    
    ResetFailStats();
    _pDBGPort       = 0;
    _pDBGPort2      = 0;
    _pNextIMU       = 0;
    _pTimer         = 0;
    _bUseGyro       = false;
    _pMas->NotifyMe(this);
    QueryChannels();
}

//! Constructor for a single IMU I2C Channel
IMU::IMU(I2C_Master* pMas, uint8_t gID, uint8_t aID)
{
    _pNextIMU   = 0;
    _gID[0]     = gID;
    _aID[0]     = aID;
    _bDualChan  = false;
    _numChans   = 1;
    _pMas       = pMas;
    _DLPF       = 0x1;
    _FullScale  = 0x1;
    _ClkSel     = 0x1;
    _Rate       = 10;
    _State      = sIdle;
    _bRun       = false;
    _pMas->NotifyMe(this);
    _pDBGPort   = 0;
    _pDBGPort2  = 0;
    _pTimer     = 0;
    ResetFailStats();    
    _bUseGyro   = false;
}

//! Constructor for a double IMU I2C Channel
IMU::IMU(I2C_Master* pMas, 
        uint8_t gID, uint8_t aID,
        uint8_t gID2, uint8_t aID2
    )
{
    _pNextIMU   = 0;
    _gID[0]     = gID;
    _aID[0]     = aID;
    _gID[1]     = gID2;
    _aID[1]     = aID2;
    _bDualChan  = true;
    _numChans   = 2;
    _pMas       = pMas;
    _DLPF       = 0x1;
    _FullScale  = 0x1;
    _ClkSel     = 0x1;
    _Rate       = 10;
    _State      = sIdle;
    _bRun       = false;
    _pMas->NotifyMe(this);
    _pDBGPort   = 0;
    _pDBGPort2  = 0;
    _pTimer     = 0;
    ResetFailStats();
    _bUseGyro   = false;
}

void IMU::NextIMU(IMUBase* pNext)
{
    _pNextIMU = pNext;
}

int IMU::BeginRead()
{
    if (_State == sWait) {
        Run(); 
    } else {
        if (_pNextIMU) {
            return _pNextIMU->BeginRead();
        }
    }
    return 0;
}

void IMU::QueryChannels()
{
    _numChans = 0;
    _bDualChan = 0;
    // Check the first, lower ID.
    int retc = _pMas->CheckID(0xD2);
    if (retc == 0) {
        _gID[_numChans] = 0xD2;
        _aID[_numChans] = 0x32; // Always high bit.
        _numChans++;
    }
    retc = _pMas->CheckID(0xD0);
    if (retc == 0) {
        _gID[_numChans] = 0xD0;
        _aID[_numChans] = 0x30; // Always low bit.
        _numChans++;
    }
    
    if (_numChans > 1) {
        _bDualChan = true;
    }
}

void IMU::SetDebugPort(DebugPort* pPort)
{
    _pDBGPort = pPort;
}

void IMU::SetDebugPort2(DebugPort* pPort)
{
    _pDBGPort2 = pPort;
}

void IMU::Reset()
{
    _pMas->end();
    _pMas->begin(400e3);
    _pMas->NotifyMe(this);
}

void IMU::ResetDevices()
{
    _pMas->Stop();
}

int IMU::ForceStartStop()
{
    return _pMas->ForceStartStop();
}

void IMU::SampleRate(uint16_t rate)
{
    // Range Limit the rate.
    if (rate < 10) {
        _Rate = 10;
    } else if (rate > 200) {
        _Rate = 200;
    } else {
        _Rate = rate;
    }
    
    uint16_t gval = 1000/rate;
    gval = gval - 1;
    
    if (pdbgserial) pdbgserial->print("Set Rate on IMU\n");
    
    SetTimerPeriod();
}

//! Perform the configuration on the connected IMUs
//! This should be done separately from the loop since
//! this process takes time and we do not want the fifos
//! to be too far out of step. 
//! Expected Context: Main
int IMU::Setup()
{
    if (_numChans == 0) return 0;
    
    _bRun = false;
    Mark marker(_pDBGPort2,pSetup);
    
    if (_State != sIdle) {
        if (pdbgserial) 
            pdbgserial->print("IMU Already Running.\n");
        return 0; // Already running
    }
    
    ResetFailStats(); // inline in header
    SetState(sConfigure); // Inline in header
    
    // Start the process with Configure
    for (int x=0; x<_numChans;x++) {
        if (pdbgserial) 
            pdbgserial->print("Configuring IMU\n");
        int retc = Configure(x);
        if (retc < 0 ) {
            // Try reset mechanisms - none of which have
            // been found that work properly yet.
            SetState(sIdle);
            if (pdbgserial) 
                pdbgserial->print("IMU Configure Failed.");
            return retc;
        }
    }
    SetState(sConfigured);
    if (pdbgserial) 
        pdbgserial->print("IMU Configured.\n");

    return 0;
}

//! Start the reading process running.
//! Run the Setup function which will do asynchronous writes
//! to all of the registers.
//! Setup the run variables and then set our state to reset the
//! fifo lengths then get going.
//! Generally the master will call all of the Setup functions first
//! so the asynchronous Writes there won't be an issue.
//! Expected Context: Main
int IMU::Start()
{
    if (_numChans == 0) return 0;
    
    Mark marker(_pDBGPort2,pWriteDn);
    
    int retc;
    if (_State != sConfigured) {
        retc = Setup();
        if (retc < 0) {
            return retc;
        }
    }
    
    cli();
    ResetTimer();
    SetState(sWait);
    ResetFailStats();
    _bDataReady[0] = false;
    _bDataReady[1] = false;
    _bRun = true;
    sei();
    return 0;
}

//! Force the state to Idle.
//! We need to consider how this works with iterrupts, but this
//! should only be called from main, below interrupt level, so I 
//! think it is okay.
int IMU::Stop()
{
    cli();
    _bRun = false;
    SetState(sIdle);
    sei();
    return 0;
}

bool IMU::Busy()
{
    return _State != sIdle;
}

//! Called by the Timer function periodically.
//! Many of the operations are chainged, meaning the completion
//! of one I2C operation, indicated by an I2C Write or Read interrupt,
//! starts the next operation in the chain. The chain checks the fifo
//! lengths of both fifos, then reads the data if there is any. If there
//! is no data yet, then the state machine enters the sWait state and the
//! next timer will initiate the chain again.
//! Expected Context: Low Lvl Timer
void IMU::Run() 
{
    if (_bRun == false) return;

    Mark marker(_pDBGPort2,pRun);

    //! Do nothing while the I2C Master is busy, the master
    //! is in the process of some operation. If the master gets
    //! hung for some reason, then we will reset and set the state
    //! to the sWait state to just start over.
    if (_pMas->busy() && BusyTimeout()) {
        Reset();
        SetState(sWait);
    }
    
    //! This block only does something in the wait state. Hopefully
    //! the rest of the logic, all driven by I2C timeouts, will keep
    //! things moving through the other states. Perhaps I should have some
    //! sort of state change timeout, as long as we are in the run mode.
    switch(_State) {
    case sWait:
        if (_bUseGyro) {
            SetState(sReadGyro1);
        } else {
            SetState(sReadAcc1);
        }
        StartTransaction();
        break;
    default:
        break;
    }
}

int IMU::StartTransaction()
{
    int retc = 0;
    switch(_State) {
    case sReadGyro1:
        RdAsync(_gID[0], 0x1B, 8);
        break;
    case sReadAcc1:
        RdAsync(_aID[0], 0x80 | 0x28, 6);
        break;
    case sReadGyro2:
        RdAsync(_gID[1], 0x1B, 8);
        break;
    case sReadAcc2:
        RdAsync(_aID[1], 0x80 | 0x28, 6);
        break;
    default:
        break;
    }
    ResetBusyTime();
    return retc;
}

//! Called when an Asynchronous I2C operation succeeds.
//! Switch on the state and perform the next appropriate action.
void IMU::ProcessTransaction()
{
    switch(_State) {
        case sReadGyro1:
            StoreGyroData(1);
            SetState(sReadAcc1);
            break;
        case sReadAcc1:
            StoreAccData(1);
            PushData(1);
            if (_bDualChan) {
                if (_bUseGyro) {
                    SetState(sReadGyro2);
                } else {
                    SetState(sReadAcc2);
                }
            } else {
                SetState(sWait);
                if (_pNextIMU) {
                    _pNextIMU->BeginRead();
                }
            }
            break;
        case sReadGyro2:
            StoreGyroData(2);
            SetState(sReadAcc2);
            break;
        case sReadAcc2:
            StoreAccData(2);
            PushData(2);
            SetState(sWait);
            if (_pNextIMU) {
                _pNextIMU->BeginRead();
            }
            break;
        default:
            break;
    }
    
    //! Start the next transaction.
    StartTransaction();
}

//! Called after we handle a Nack, Bus Error or ArbLost
//! Attempt to fix something, then return to the previous state
//! and give it another go. We will basically keep doing this
//! forever until the Manager says stop. 
//! All error or fail recovery goes through here.. I have finally
//! gotten things cleaned up enough so that I have a central location
//! for error attempts.
void IMU::FailRecovery()
{
    switch(_failType) {
    case fNone:
        break;
    case fNack:
        if (_nackCount <7) {
            _pMas->WigglePin(10, 0,1);
        } else if (_nackCount < 10) {
            Reset();
            _pMas->WigglePin(10,0,1);
        }
        SetState(_previousState);
        break;
    case fBusErr:
        if (_failCount < 5) {
            Reset();
        }
        SetState(_previousState);
        break;
    case fArbLost:
        if (_failCount < 5) {
            Reset();
        }
        SetState(_previousState);
        break;
    }
    
    StartTransaction();
}

//! Called by the Master when the write is complete.
//! Requires registration
//! Expected Context: Med Lvl I2C Int.
void IMU::I2CWriteDone()
{
    if (_bRun == false) return;

    Mark marker(_pDBGPort2,pWriteDn);
    ResetBusyTime();

    ResetFailStats();
    ProcessTransaction();
}
  
//! Called be the master when the read is complete.
//! Requires registration
//! Expected Context: Med Lvl I2C Int.
void IMU::I2CReadDone()
{
    if (_bRun == false) return;

    Mark marker(_pDBGPort2,pReadDn);
    ResetBusyTime();
    
    ResetFailStats();
    ProcessTransaction();
}

//! Called by the I2C Master if we get a NAck.
//! Current idea is the repeat the current command until
//! it works since I am seeing that sometimes "Nacks" are temporary
//! So a retry is best. Other types of failures may indicate a need
//! for more desperate action - but those will probably be BusError
//! or Arb Lost commands.
void IMU::I2CNack()
{
    if (_bRun == false) return;
    
    ResetBusyTime();
    {
        Mark marker(_pDBGPort2,pBusErr);
        _delay_us(3);
    }
    Mark marker(_pDBGPort2,pNack);

    ++_nackCount;
    
    //! I temporarily set this to ErrRecover and then delay.
    //! If I am re-starting, then I will go right back, but I
    //! will at least trigger the Logic Analyzer
    SetState(sErrRecover);
    _delay_us(5);

    //! Re-start the same transaction.
    if (_nackCount < 5) {
        //! Retry the current transaction until we are sure it won't work.
        SetState(_previousState);
        StartTransaction();
    } else if (_nackCount < 10) {
        _failType = fNack;
        FailRecovery();
    } else {
        //! Keep failing.. just stop.
        Stop();
    }
}

//! Called by I2C Master when a Bus error occurs. This means some
//! non-I2C compliant event occured. Normally, this is going to mean
//! that some glitch occured on the I2C Bus.
void IMU::I2CBusError()
{
    if (_bRun == false) return;

    ResetBusyTime();
    {
        Mark marker(_pDBGPort2,pBusErr);
        _delay_us(3);
    }
    Mark marker(_pDBGPort2,pBusErr);
    //! For a first pass, I am going to just try and return to Wait
    //! state - this way the timer will take back over and re-try
    //! some operation.

    _bFailDetected = true;
    ++_failCount;
    _failType = fBusErr;

    //! I temporarily set this to ErrRecover and then delay.
    //! If I am re-starting, then I will go right back, but I
    //! will at least trigger the Logic Analyzer
    SetState(sErrRecover);
    _delay_us(5);
    if (_failCount > 10) {
        FailRecovery();
    } else {
        SetState(_previousState);
        StartTransaction();
    }
}

//! This occurs if the Arbitration is lost.
//! If the I2C is in master mode, and it detects that it cannot control
//! the state of the data line, i.e. it wants to set a HIGH but the line
//! stays low, then this error occurs.
void IMU::I2CArbLost()
{
    if (_bRun == false) return;

    ResetBusyTime();
    {
        Mark marker(_pDBGPort2,pBusErr);
        _delay_us(3);
    }
    Mark marker(_pDBGPort2,pArbLost);
    //! For a first pass, I am going to just try and return to Wait
    //! state - this way the timer will take back over and re-try
    //! some operation.
    _bFailDetected = true;
    ++_failCount;
    _failType = fArbLost;

    //! I temporarily set this to ErrRecover and then delay.
    //! If I am re-starting, then I will go right back, but I
    //! will at least trigger the Logic Analyzer
    SetState(sErrRecover);
    _delay_us(5);
    if (_failCount > 10) {
        FailRecovery();
    } else {
        SetState(_previousState);
        StartTransaction();
    }
}

bool IMU::DataReady()
{
    if (_numChans == 0) return true;
    
    bool bReady = false;
    cli();
    if (_bDualChan) {
        bReady =  _bDataReady[0] && _bDataReady[1];
    } else {
        bReady =  _bDataReady[0];
    }
    sei();
    return bReady;
}

//! Store the gyro read data while I get the Accelerometer data
//! I have the index there in case I need it later..
void IMU::StoreGyroData(uint8_t idx)
{
    _pMas->ReadData(&_dataBuffer[idx-1][0],8);
}

//! Add the accelerometer data to the buffer
//! I have the index there in case I need it later..
void IMU::StoreAccData(uint8_t idx)
{
    _pMas->ReadData(&_dataBuffer[idx-1][8],6);
}

//! Push the data in the temporary buffer onto the appropriate fifo
void IMU::PushData(uint8_t idx)
{
    _bDataReady[idx-1] = true;
}

//! Retrieve packet data from the stored packets.
//! If no packet data exists, this function will
//! fill the data pointer with null data - this way
//! any host software can continue, even if data is bad.    
uint8_t* IMU::GetPacketData(uint8_t* pData)
{
    if (_numChans == 0) return pData;

    cli();

    *pData++ = 0xa5;
    *pData++ = 0x5a;
    if (_State == sIdle) {
        if (_failType == fNack) {
            memset(pData,'N',IMUPacket::PacketLen);
        } else {
            memset(pData,'I',IMUPacket::PacketLen);
        }
        pData += IMUPacket::PacketLen;
    } else if (_bDataReady[0] == true) {
        memcpy(pData,&_dataBuffer[0][0],IMUPacket::PacketLen);
        _bDataReady[0] = false;
        pData += IMUPacket::PacketLen;
    } else {
        memset(pData,0,IMUPacket::PacketLen);
        pData += IMUPacket::PacketLen;
    }

    if (_bDualChan) {
        *pData++ = 0xa5;
        *pData++ = 0x5a;
        if (_State == sIdle) {
            if (_failType == fNack) {
                memset(pData,'N',IMUPacket::PacketLen);
            } else {
                memset(pData,'I',IMUPacket::PacketLen);
            }
            pData += IMUPacket::PacketLen;
        } else if (_bDataReady[1] == true) {
            memcpy(pData,&_dataBuffer[1][0],IMUPacket::PacketLen);
            _bDataReady[1] = false;
            pData += IMUPacket::PacketLen;
        } else {
            memset(pData,0,IMUPacket::PacketLen);
            pData += IMUPacket::PacketLen;
        }
    }
    sei();
    return pData;
}

// Diagnostic Routines
void IMU::CheckIDs(HardwareSerial* pSerial)
{
    char buffer[50];
    for (int x=0;x<_numChans;x++) {
        int retc = _pMas->CheckID(_gID[x]); 
        if (retc == 0) {
            sprintf(buffer,"Gyro%d (0x%x):Ack.\n",x,_gID[x]);
            pSerial->print(buffer);
        } else {
            sprintf(buffer,"Gyro%d (0x%x):NAck (%d).\n",x,_gID[x],retc);
            pSerial->print(buffer);
        }
        Wr(_gID[x], 0x3D, 0x8);
        retc = _pMas->CheckID(_aID[x]); 
        if (retc == 0) {
            sprintf(buffer,"Acc%d (0x%x):Ack.\n",x,_aID[x]);
            pSerial->print(buffer);
        } else {
            sprintf(buffer,"Acc%d (0x%x):NAck (%d).\n",x,_aID[x],retc);
            pSerial->print(buffer);
        }
    }
}

//! Set Timer object to use for the main timer tick.
//! This timer needs to be fast enough to send off packets
//! at the rate configured. So, if 200Hz is the rate, then
//! this timer must run at 1 500us rate, etc. The default will
//! be to run at 500us, then the timer can be slowed down
//! if a slower rate is used, just to avoid as much overhead.
//! The CPU Clock runs at 32Mhz, so the main timer clock is running
//! at 32/64 or 500us period. 
//! Timer set to same interrupt level as the I2C so that those
//! interrupts won't ever stomp on each other.
void IMU::SetTimer(TimerCntr* pTimer)
{
    _pTimer = pTimer;
    
    //! This will be 2us period
    _pTimer->ClkSel(TC_CLKSEL_DIV64_gc);
    SetTimerPeriod();
    _pTimer->CCEnable(0);
    _pTimer->WaveformGenMode(TC_WGMODE_NORMAL_gc);
    _pTimer->EventSetup(TC_EVACT_OFF_gc,TC_EVSEL_OFF_gc);
    _pTimer->IntLvlA(0,1);
    _pTimer->IntLvlB(0);
    _pTimer->Notify(this,0);
}

//! Reset the counter value. The master uses this to get all of the
//! timers operating at the same time.
void IMU::ResetTimer()
{
    if (_pTimer) _pTimer->Counter(0);
}

void IMU::SetTimerPeriod()
{
    // Adjust the timer function to fire 5X faster
    // than the rate. At 200Hz, this will be 2Khz or 
    // every 500us. 
    // We set the timer to go off 5 times per IMU period.
    // This should range from 20ms for 10Hz, and 1 ms for 200
    // **** NoFifo
    // Set timer to fire at the rate.
    //unsigned long timerTicks = 100000/_Rate;
    unsigned long timerTicks = 500000/_Rate;
    if (timerTicks > 65000) {
        timerTicks = 65000;
    }
    if (_pTimer) _pTimer->Period(timerTicks);
}

/** @name Interrupt Handlers
 
These handlers receive interupts from the Timer class.
We registered to received these calls with the Notify function. In some
cases we might have 2 or more objects that will send us interrupt notifications,
in this case we give each object an ID that is passed back so that we
know which one caused the interrupt.

For the IMUManager, there is only a single Timer, so the ID is always 0.
    
*/
//@{

//! Timer Error - ignored.
void IMU::err(uint8_t id)
{
}

//! Timer Overflow Interrupt.
//! Overflow fires when the timer reaches the top period value.
//! This is setup to fire when we get a timer tick, with a default rate of 500us
//! IMUManager has only one Timer so the ID is not needed.
void IMU::ovf(uint8_t id)
{
    Run();
}

//! Timer Capture Compare - not used.
void IMU::ccx(uint8_t id,uint8_t idx)
{
}

//@}

int IMU::Wr(uint8_t ID, uint8_t addr, uint8_t data)
{
    static uint8_t  bytes[2];
    bytes[0] = addr;
    bytes[1] = data;
    return _pMas->WriteSync(ID, &bytes[0],2);
}

int IMU::Rd(uint8_t ID, uint8_t addr, uint8_t cnt, uint8_t* pData)
{
    // Only a single write, the address, then read data.
    int retc = _pMas->WriteReadSync(ID, &addr, 1, cnt);
    if ( retc < 0 ) {
        return retc;
    }
    return _pMas->ReadData(pData,cnt);
}

int IMU::WrAsync(uint8_t ID, uint8_t addr, uint8_t data)
{
    static uint8_t  bytes[2];
    bytes[0] = addr;
    bytes[1] = data;
    sprintf(buffer,"WrAsync to %d\n",ID);
    if (pdbgserial) pdbgserial->print(buffer);
    return _pMas->WriteRead(ID, &bytes[0],2,0);
}

int IMU::RdAsync(uint8_t ID, uint8_t addr, uint8_t cnt)
{
    // Only a single write, the address, then read data.
    return _pMas->WriteRead(ID, &addr, 1, cnt);
}

void IMU::ReadWord(uint16_t* pData)
{
    static uint8_t buffer[2];
    _pMas->ReadData(&buffer[0],2);
    *pData = (buffer[0] << 8 | buffer[1]);
}

//! Configure the Gyro and Accelerometer device.
//! The input parameter selects the first or second channel.
//! Build an array of RegWrite types so that I can check
//! the return code of each write to insure they all pass.
int IMU::Configure(uint8_t idx)
{
    // Value for the sensor register
    uint16_t gval = 1000/_Rate;
    gval = gval - 1;

    int retc;
    
    RegWriteType    config[] = {
    //    // Turn on pass-through
        { _gID[idx], 0x3D, 0x0F },
    //    
    //    // Init the Accelerometer.
        { _aID[idx], 0x20, 0x37},
        { _aID[idx], 0x21, 0x0},
        { _aID[idx], 0x22, 0x0},
        { _aID[idx], 0x23, 0x80 | 0x40},
        { _aID[idx], 0x24, 0x00},
    //    
    //    // Set offsets to zero
        { _gID[idx], 0x0C, 0x00},
        { _gID[idx], 0x0D, 0x00},
        { _gID[idx], 0x0E, 0x00},
        { _gID[idx], 0x0F, 0x00},
        { _gID[idx], 0x10, 0x00},
        { _gID[idx], 0x11, 0x00},
    //    
    //    // Configure registers.
        { _gID[idx], 0x12, 0xff},                     // Enable all outputs to to the fifo
        { _gID[idx], 0x13, 0x00},
    //    { _gID[idx], 0x14, _aID[idx] >> 1},             // Set slave address of ACC
        { _gID[idx], 0x15, gval},                     // Set sample rate
        { _gID[idx], 0x16, _DLPF | _FullScale << 3},
        { _gID[idx], 0x17, 0x00},
    //    { _gID[idx], 0x18, 0x80 | 0x28},              // Set burst address for Accelerometer, enable auto addr increment.
        { _gID[idx], 0x3E, _ClkSel},
    };
    
    uint8_t nItems = sizeof(config)/sizeof(RegWriteType);
    for (int idx = 0;idx <nItems;idx++) {
        retc = Wr(config[idx].ID, 
            config[idx].Addr,
            config[idx].Data);
        ResetBusyTime();
        
        if (retc < 0) {
            return retc; // _configOkay will be false;
        }
    }
    
    return 0;
}



