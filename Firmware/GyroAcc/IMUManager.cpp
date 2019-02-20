#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "HardwareSerial.h"
#include "TimerCore.h"
#include "TimerCntr.h"

#include "IMU.h"
#include "IMUManager.h"

extern HardwareSerial* pdbgserial;
static uint8_t  buffer[255];

#define ALL_IMU(func) \
    for (int x=0;x<4;x++) { \
        if (_pIMU[x]) {         \
            _pIMU[x]->func();   \
        }   \
    }

#define ALL_IMUP(func,param) \
    for (int x=0;x<4;x++) { \
        if (_pIMU[x]) {         \
            _pIMU[x]->func(param);   \
        }   \
    }

#define ALL_IMURET(func) \
    int retc =0; \
    for (int x=0;x<4;x++) { \
        if (_pIMU[x]) {         \
            retc = _pIMU[x]->func();   \
            if (retc < 0) { \
                return retc;\
            }\
        }   \
    }\
    return retc;
    
#define ALL_IMUBOOL(func) \
    bool bReady = true; \
    for (int x = 0;x<4;x++) {   \
        if (_pIMU[x]) { \
            if (!_pIMU[x]->func()) { \
                bReady = false; \
            }   \
        }   \
    }   \
    return bReady;

IMUManager::IMUManager(HardwareSerial* pSerial)
{
    int x;
    for (x =0;x<4;x++) {
        _pIMU[x] = 0;
    }
    
    _nIMUs          = 0;
    _State          = sIdle;
    _nStreamWDCounter = 0;
    _packetId       = 0;
    _pBlueLed       = 0;
    _bLedState      = false;

    _pTimer         = 0;
    _pSerial        = pSerial;
    _pDBGPort       = 0;
    _sampleRate     = 10;
    _maxMillisPerPacket = 1000;
    _lastSendMillis = 0;
} 

void IMUManager::SetBlueLed(PORT_t* port, uint8_t Pin)
{
    _pBlueLed   = port;
    _LedPin     = Pin;
    
    _pBlueLed->DIRSET = _LedPin;
    _pBlueLed->OUTSET = _LedPin;
    _bLedState        = false;
}

void IMUManager::LedOn()
{
    _pBlueLed->OUTCLR   = _LedPin;
    _bLedState          = true;
}

void IMUManager::LedOff()
{
    _pBlueLed->OUTSET   = _LedPin;
    _bLedState          = false;
}

//! This function is called once per stream data.
//! We are going to toggle this once for every 3
//! streams, this way it will be high/low each alternate
//! frame. This works if our packet rate is 3X the rate
//! of the camera. The camera is actually 59.94FPS 
void IMUManager::PacketLedIndicator()
{
    if ((_packetId % 6) == 0) {
        ToggleLed();
    }
}

//! Toggle the current Led State.
void IMUManager::ToggleLed()
{
    if ( _bLedState) {
        _bLedState = false;
        _pBlueLed->OUTCLR = _LedPin;
    } else {
        _bLedState = true;
        _pBlueLed->OUTSET = _LedPin;
    }
}

//! Flash the LED in a pattern to indicate stream starting
void IMUManager::ShowLedStart()
{
    _pBlueLed->OUTCLR = _LedPin;
    _delay_ms(10*17); // One Frame
    _pBlueLed->OUTSET = _LedPin;
    _delay_ms(5*17); // One Frame
    _pBlueLed->OUTCLR = _LedPin;
    _delay_ms(10*17); // One Frame
    _pBlueLed->OUTSET = _LedPin;
    _delay_ms(17); // One Frame
    _bLedState        = false;
}

//! Flash the LED in a pattern to indicate stream stopped
void IMUManager::ShowLedStop()
{
    _pBlueLed->OUTCLR = _LedPin;
    _delay_ms(5*17); // One Frame
    _pBlueLed->OUTSET = _LedPin;
    _delay_ms(5*17); // One Frame
    _pBlueLed->OUTCLR = _LedPin;
    _delay_ms(5*17); // One Frame
    _pBlueLed->OUTSET = _LedPin;
    _delay_ms(5*17); // One Frame
    _pBlueLed->OUTCLR = _LedPin;
    _bLedState        = false;
}

void IMUManager::SampleRate(uint16_t rate)
{
    // Range Limit the rate.
    if (rate < 10) {
        _sampleRate = 10;
    } else if (rate > 200) {
        _sampleRate = 200;
    } else {
        _sampleRate = rate;
    }

    ALL_IMUP(SampleRate,_sampleRate);
    
    
    if (pdbgserial) {
        char buffer[50];
        sprintf(buffer,"Sample rate set to %d\n", _sampleRate);
        pdbgserial->print(buffer);
    }

    // Note: I had this set to 2X the sample rate (in milliseconds),
    // but that was a problem. The IMU's wait until there is 2X the
    // amount of data in the fifo, so that takes 2X the time, and we
    // would time out first time around.. bad idea.
    _maxMillisPerPacket = 3 * 1000/_sampleRate;
    
    SetTimerPeriod();
}

int IMUManager::AddIMU(IMUBase* pIMU)
{
    for (int x=0;x<4;x++) {
        if (_pIMU[x] == 0) {
            // Add into first empty spot
            _pIMU[x] = pIMU;
            if (x > 0) {
                _pIMU[x-1]->NextIMU(pIMU);
            }
            _nIMUs++;
            return x;
        }
    }
    
    // Seems we are full!!
    return -1;
}

int IMUManager::Setup()
{
    ALL_IMURET(Setup);
}

int IMUManager::Start()
{
    ALL_IMURET(Start);
}

void IMUManager::Stop()
{
    ALL_IMU(Stop);
    _State = sIdle;
    ShowLedStop();  
    LedOff();
}

void IMUManager::Reset()
{
    ALL_IMU(Reset);
    
    _State = sIdle; 
    _packetId       = 0;
}

void IMUManager::ResetDevices()
{
    ALL_IMU(ResetDevices);
}

void IMUManager::ForceStartStop()
{
    ALL_IMU(ForceStartStop);
}

// Diagnostic Routines
void IMUManager::CheckIDs(HardwareSerial* pSerial,int idx)
{
    if (idx < 0) {
        ALL_IMUP(CheckIDs,pSerial);
    } else {
        if (_pIMU[idx]) {
            _pIMU[idx]->CheckIDs(pSerial);
        }
    }
}

//! Check if all IMU's have data.
bool IMUManager::DataReady()
{
    ALL_IMUBOOL(DataReady)
}

void IMUManager::IMUUseGyro(bool bEn)
{
    ALL_IMUP(UseGyro,bEn);
}

//! Start Streaming Data.
//! If the current stream count == 0, then
//! re-configure all of the IMU's and make sure
//! everything is okay before we start.
//! If the count is > 0, then we are just
//! adding more streams to the count so continue.
int IMUManager::StreamStart(bool bUseGyro)
{
    if (_State != sIdle ) {
        Stop();
    }
    
    IMUUseGyro(bUseGyro);
    
    int retc = Setup();
    if (retc < 0) {
        return retc;
    }
    
    retc = Start();
    
    if (retc < 0) {
        return retc;
    }
    
    ShowLedStart();
    _LedCounter = 0;
    _packetId       = 0;
    _nStreamWDCounter = 20;
    
    SetState(sDataWait); // Jump ahead a state.
    ResetDataReadyTO();
    return 0;
}

void IMUManager::StreamWatchdog()
{
    _nStreamWDCounter = 20;
}

//
//  Iterate over the embedded IMU objects, retrieve results
//  as needed. Do this in a 2-pass fashion, so that we do
//  IMU 1 on each interface first, then IMU2 on any interfaces
//  that have 2 IMU devices on them. After this is all done,
//  we should have all of the required IMU data, then we can
//  initiate a packet send to the host with as much as six 
//  IMU's worth of data!
//
int IMUManager::Loop()
{
    switch(_State) {
    case sIdle:
        break;
    case sDataWait:
        if (DataReady()) {
            ResetDataReadyTO();
            _State = sDataReady;
        } else if (DataReadyTimeout()) {
            ResetDataReadyTO();
            _State = sDataTimeout;
        }
        break;
    case sDataReady:
        PacketLedIndicator();
        if (_nStreamWDCounter == 0) {
            DiscardData();
            _State = sDataWait;
        } else {
            --_nStreamWDCounter;
            SendPacket(false);
            _State = sDataWait;
        }
        break;
    case sDataTimeout:
        PacketLedIndicator();
        if (_nStreamWDCounter == 0) {
            DiscardData();
            _State = sDataWait;
        } else {
            --_nStreamWDCounter;
            SendPacket(true);
            _State = sDataWait;
        }
        break;
    }
    
    return 0;
}

void IMUManager::Run()
{
    if (_State != sIdle) {
        // Start the IMU's going one at a time...
        if (_pIMU[0]) {
            _pIMU[0]->BeginRead();
        }
    }
}

//! Send a response packet with stream data.
//! Retreive stream data from each of the child IMU
//! devices. Each IMU updates the pointer so that we
//! do not really know if they send back 1 or 2 IMU's
//! worth of information, but it does not matter since
//! we use pointer diff to determine how much data to return.
//! Send the appropriate header, then the packet data, then
//! a fooder including a CRC *not yet implemented though *
void IMUManager::SendPacket(bool bTimeout)
{
    uint8_t*    pPacket = &_dataPacket[0];
    if (true || !bTimeout) {
        for (int x = 0;x<4;x++) {
            if (_pIMU[x]) {
                // This puts the data at the pointer,
                // then returns the end of the data.
                // This might be 2*14 or 1*14
                pPacket = _pIMU[x]->GetPacketData(pPacket);
            }
        }
    }
    // Packet format:
    // SNP header
    // byte: length of packet
    // byte: packet type (0xB7)
    // byte(s): length bytes
    // bytes(2): 2 byte CRC
    // string: END
    // newline
    uint8_t size = pPacket - &_dataPacket[0];
    buffer[0] = 'S';
    buffer[1] = 'N';
    buffer[2] = 'P';
    buffer[3] = 0xB7;
    buffer[4] = _packetId++;
    buffer[5] = size;
    memcpy(&buffer[6],&_dataPacket[0],size);
    // Compute CRC -- someday
    uint16_t crc = 0xaf5a;
    uint8_t crchi = (crc >> 8) & 0xff;
    uint8_t crclo = crc & 0xff;
    buffer[6+size]   = _nStreamWDCounter;
    buffer[6+size+1]   = crchi;
    buffer[6+size+2] = crclo;
    sprintf((char*)&buffer[6+size+3],"END\n");
    _pSerial->write(&buffer[0],6+size+3+4);
}

void IMUManager::DiscardData()
{
    uint8_t*    pPacket = &_dataPacket[0];
    for (int x = 0;x<4;x++) {
        if (_pIMU[x]) {
            // This puts the data at the pointer,
            // then returns the end of the data.
            // This might be 2*14 or 1*14
            pPacket = _pIMU[x]->GetPacketData(pPacket);
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
void IMUManager::SetTimer(TimerCntr* pTimer)
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

void IMUManager::SetTimerPeriod()
{
    // Adjust the timer function to fire 5X faster
    // than the rate. At 200Hz, this will be 2Khz or 
    // every 500us. 
    // We set the timer to go off 5 times per IMU period.
    // This should range from 20ms for 10Hz, and 1 ms for 200
    // **** NoFifo
    // Set timer to fire at the rate.
    //unsigned long timerTicks = 100000/_Rate;
    unsigned int timerTicks = 500000/_sampleRate;
    if (timerTicks > 65000) {
        timerTicks = 65000;
    }
    // This is a special case. If I set the rate to 180,
    // then this will assume I am trying to sync with the camera
    // which has a frame rate of 59.94. 3X this is 179.82.
    // Setting this timer ticks value will put our IMU rate
    // close to 3x the frame rate of the camera, which is what
    // we want.
    if (_sampleRate == 180) {
        timerTicks = 2780;
    }
    if (pdbgserial) {
        char buffer[50];
        sprintf(buffer,"Timer Period:%u\n",timerTicks);
        pdbgserial->print(buffer);
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
void IMUManager::err(uint8_t id)
{
}

//! Timer Overflow Interrupt.
//! Overflow fires when the timer reaches the top period value.
//! This is setup to fire when we get a timer tick, with a default rate of 500us
//! IMUManager has only one Timer so the ID is not needed.
void IMUManager::ovf(uint8_t id)
{
    Run();
}

//! Timer Capture Compare - not used.
void IMUManager::ccx(uint8_t id,uint8_t idx)
{
}

//@}

