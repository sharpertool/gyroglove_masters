
#ifndef TimerCntr_h
#define TimerCntr_h

class TimerNotify
{
public:
    virtual void err(uint8_t id) = 0;
    virtual void ovf(uint8_t id) = 0;
    virtual void ccx(uint8_t id,uint8_t idx) = 0;
};

class TimerCntr
{
    // Since TC0 is a super-set, use this for the 
    // default, but then we will override this in the
    // concrete classes.
    TC0_t*          _pTC;
    bool            _bTC1;
    TimerNotify*    _pNotifyClient;
    uint8_t         _pNotifyClientID;
    
public:
    
    TimerCntr(TC0_t* pTC);
    TimerCntr(TC1_t* pTC);
    ~TimerCntr();
    
    void Notify(TimerNotify* pClient, uint8_t id);
    
    void ovf();
    void err();
    void ccx(uint8_t idx);
    
    //! Set the clock source for this timer in CTRLA
    void ClkSel(TC_CLKSEL_t clksel);
    
    //! Set the state of the 4 CCxEN bits in
    //! CTRLB. The 4 lower bits of mask will
    //! enable D, C, B or A if set to 1
    void CCEnable(uint8_t mask);
    
    //! Set the Waveform generator mode. Normal disables
    //! waveform generation.
    void WaveformGenMode(TC_WGMODE_t wgmode);
    
    void EventSetup(TC_EVACT_t act, TC_EVSEL_t src);
    
    void IntLvlA(uint8_t errlvl, uint8_t ovflvl);
    
    void IntLvlB(uint8_t val);
    
    void Counter(uint16_t newVal);
    uint16_t Counter();
    
    void Period(uint16_t newPer);
    uint16_t Period();
    
    void SetRate(uint32_t rateHz);
    
    void CCReg(uint8_t idx, uint16_t newVal);
    uint16_t CCReg(uint8_t idx);
    
};


#endif


