
#ifndef MYDRIVER_H
#define MYDRIVER_H

#include <avr/io.h>
#include "TimerCntr.h"


class MyDriver : public TimerNotify 
{

public:
    unsigned char aValue;
    bool bLedState;

    volatile PORT_t*    aPort;
    volatile PORT_t*    bPort;
    
    unsigned long   _Rate;
    TimerCntr*      _pTimer;
    
    MyDriver() {
        bLedState = false;
        bPort = (volatile PORT_t*)&PORTC;
        bPort->DIRSET = PIN6_bm;
        bPort->OUTSET = PIN6_bm; // Off when high
        
        _Rate = 2;
    }

    void toggleLed() {
        if (bLedState) {
            // Set State On
            bLedState = false;
            bPort->OUTCLR = PIN6_bm;
        } else {
            bLedState = true;
            bPort->OUTSET = PIN6_bm;
        }
    }
    
    //! Set Timer object to use for the main timer tick.
    //! This timer needs to be fast enough to send off packets
    //! at the rate configured. So, if 200Hz is the rate, then
    //! this timer must run at 1 500us rate, etc. The default will
    //! be to run at 500us, then the timer can be slowed down
    //! if a slower rate is used, just to avoid as much overhead.
    //! The CPU Clock runs at 32Mhz, so the main timer clock is running
    //! at 32/8 or 4Mhz. 
    virtual void SetTimer(TimerCntr* pTimer)
    {
        _pTimer = pTimer;
        
        //! This will be 2us period
        _pTimer->ClkSel(TC_CLKSEL_DIV256_gc);
        SetTimerPeriod();
        _pTimer->CCEnable(0);
        _pTimer->WaveformGenMode(TC_WGMODE_NORMAL_gc);
        _pTimer->EventSetup(TC_EVACT_OFF_gc,TC_EVSEL_OFF_gc);
        _pTimer->IntLvlA(0,2);
        _pTimer->IntLvlB(0);
        _pTimer->Notify(this,0);
    }
    
    virtual void SetTimerPeriod()
    {
        // Adjust the timer function to fire 10X faster
        // than the rate. At 200Hz, this will be 2Khz or 
        // every 500us. 
        unsigned long timerTicks = 31250/_Rate;
        _pTimer->Period(timerTicks);
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
    virtual void err(uint8_t id)
    {
    }
    
    //! Timer Overflow Interrupt.
    //! Overflow fires when the timer reaches the top period value.
    //! This is setup to fire when we get a timer tick, with a default rate of 500us
    //! IMUManager has only one Timer so the ID is not needed.
    virtual void ovf(uint8_t id)
    {
        toggleLed();
    }
    
    //! Timer Capture Compare - not used.
    virtual void ccx(uint8_t id,uint8_t idx)
    {
    }
    
    //@}

};

#endif
