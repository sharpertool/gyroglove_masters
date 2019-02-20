

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "TimerCore.h"
#include "TimerCntr.h"

// Generate all of the ISR handlers.. hook them up to a class if/when a class
// is instantiated for a particular USART.
#define TC0_ISR_DEF(tc) \
static TimerCntr*  tc##cp = 0;\
ISR(tc##_ERR_vect) {\
    if (tc##cp) tc##cp->err();\
}\
ISR(tc##_OVF_vect) {\
    if (tc##cp) tc##cp->ovf();\
}\
ISR(tc##_CCA_vect) {\
    if (tc##cp) tc##cp->ccx(0);\
}\
ISR(tc##_CCB_vect) {\
    if (tc##cp) tc##cp->ccx(1);\
}\
ISR(tc##_CCC_vect) {\
    if (tc##cp) tc##cp->ccx(2);\
}\
ISR(tc##_CCD_vect) {\
    if (tc##cp) tc##cp->ccx(3);\
}

TC0_ISR_DEF(TCC0);
TC0_ISR_DEF(TCD0);

// The TimerCore.cpp will use TCE0 and TCE1 if
// TCF1 is not defined.. Hence, I need to NOT define
// It here. 
#if not defined(TIMERCORE_USE_TCE0)
TC0_ISR_DEF(TCE0);
#endif

#if not defined(TIMERCORE_USE_TCF0)
TC0_ISR_DEF(TCF0);
#endif

#define TC1_ISR_DEF(tc) \
static TimerCntr*  tc##cp = 0;\
ISR(tc##_ERR_vect) {\
    if (tc##cp) tc##cp->err();\
}\
ISR(tc##_OVF_vect) {\
    if (tc##cp) tc##cp->ovf();\
}\
ISR(tc##_CCA_vect) {\
    if (tc##cp) tc##cp->ccx(0);\
}\
ISR(tc##_CCB_vect) {\
    if (tc##cp) tc##cp->ccx(1);\
}

TC1_ISR_DEF(TCC1);
TC1_ISR_DEF(TCD1);
#if not defined(TIMERCORE_USE_TCE1)
TC1_ISR_DEF(TCE1);
#endif

#if not defined(TIMERCORE_USE_TCF1)
#ifdef TCF1
TC1_ISR_DEF(TCF1);
#endif
#endif

static void SetPointer(TC0_t* tc,TimerCntr* pTC)
{
    // Register this object with the appropriate
    // pointer so that the ISR routines can call p
    // class.
    if (tc == &TCC0) {
        TCC0cp = pTC;
    } else if (tc == &TCD0) {
        TCD0cp = pTC;
#if not defined(TIMERCORE_USE_TCE0)
    } else if (tc == &TCE0) {
        TCE0cp = pTC;
#endif
#if not defined(TIMERCORE_USE_TCF0)
    } else if (tc == &TCF0) {
        TCF0cp = pTC;
#endif
    }
}

static void SetPointer(TC1_t* tc,TimerCntr* pTC)
{
    // Register this object with the appropriate
    // pointer so that the ISR routines can call p
    // class.
    if (tc == &TCC1) {
        TCC1cp = pTC;
    } else if (tc == &TCD1) {
        TCD1cp = pTC;
#if not defined(TIMERCORE_USE_TCE1)
    } else if (tc == &TCE1) {
        TCE1cp = pTC;
#endif
#if not defined(TIMERCORE_USE_TCF1)
#if defined(TCF1)
    } else if (tc == &TCF1) {
        TCF1cp = pTC;
#endif
#endif
    }
}

TimerCntr::TimerCntr(TC0_t*  pTC)
{
    _pTC = pTC;
    _bTC1 = false;
    _pNotifyClient = 0;
    _pNotifyClientID = 0;
    SetPointer(pTC,this);
}

TimerCntr::TimerCntr(TC1_t*  pTC)
{
    _pTC = (TC0_t*)pTC;
    _bTC1 = true;
    _pNotifyClient = 0;
    _pNotifyClientID = 0;
    SetPointer(pTC,this);
}

TimerCntr::~TimerCntr()
{
    if (_bTC1) {
        SetPointer((TC1_t*)_pTC,0);
    } else {
        SetPointer(_pTC,0);
    }
}


void TimerCntr::ClkSel(TC_CLKSEL_t clksel)
{
    _pTC->CTRLA = clksel; 
}


void TimerCntr::CCEnable(uint8_t mask)
{
    _pTC->CTRLB = ((_pTC->CTRLB & 0x0F) | (mask << 4));
}


void TimerCntr::WaveformGenMode(TC_WGMODE_t wgmode)
{
    _pTC->CTRLB = ((_pTC->CTRLB & 0xF0) | wgmode);
}


void TimerCntr::EventSetup(TC_EVACT_t act, TC_EVSEL_t src)
{
    _pTC->CTRLD = act | src;
}


void TimerCntr::IntLvlA(uint8_t errlvl, uint8_t ovflvl)
{
    _pTC->INTCTRLA = (errlvl & 0x3) << 2 | (ovflvl & 0x3);
}

void TimerCntr::IntLvlB(uint8_t val)
{
    _pTC->INTCTRLB = val;
}

void TimerCntr::Counter(uint16_t newVal)
{
    _pTC->CNT = newVal;
}

uint16_t TimerCntr::Counter()
{
    return _pTC->CNT;
}


void TimerCntr::Period(uint16_t newVal)
{
    _pTC->PER = newVal;
}

uint16_t TimerCntr::Period()
{
    return _pTC->PER;
}


void TimerCntr::CCReg(uint8_t idx, uint16_t newVal)
{
    if (idx == 0) {
        _pTC->CCA = newVal;
    } else if (idx == 1) {
        _pTC->CCB = newVal;
    } else if (!_bTC1 && idx == 2) {
        _pTC->CCC = newVal;
    } else if (!_bTC1 && idx == 3) {
        _pTC->CCD = newVal;
    }
}

uint16_t TimerCntr::CCReg(uint8_t idx)
{
    if (idx == 0) {
        return _pTC->CCA;
    } else if (idx == 1) {
        return _pTC->CCB;
    } else if (!_bTC1 && idx == 2) {
        return _pTC->CCC;
    } else if (!_bTC1 && idx == 3) {
        return _pTC->CCD;
    }
    return 0;
}

void TimerCntr::Notify(TimerNotify* pClient,uint8_t id)
{
    _pNotifyClient = pClient;
    _pNotifyClientID = id;
}

void TimerCntr::err()
{
    if (_pNotifyClient)  
        _pNotifyClient->err(_pNotifyClientID);
}

void TimerCntr::ovf()
{
    if (_pNotifyClient) 
        _pNotifyClient->ovf(_pNotifyClientID);
}

void TimerCntr::ccx(uint8_t idx)
{
    if (_pNotifyClient) 
        _pNotifyClient->ccx(_pNotifyClientID,idx);
}

void TimerCntr::SetRate(uint32_t rateHz){
    //add an auto prescaler using 32 bit array
}



