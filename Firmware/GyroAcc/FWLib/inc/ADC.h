#ifndef ADC_H
#define ADC_H

#include <stdlib.h>
#include <avr/io.h>


//! CFAADC Class - interface to the ATXMega ADC Logic
class ADC
{
private:
    
protected:
    ADC_t*              _pADC;
    uint8_t             _dmasel;
    bool                _bSigned;
    bool                _bFreeRun;
    
public:
    //! Generally I like to keep these private and make an
    //! accessor, but I don't feel like making this many
    //! accessors right now.. Nothing to really be gained for now.
    ADC_CH_GAIN_t       chGain[4];
    ADC_CH_INPUTMODE_t  chInput[4];
    ADC_CH_MUXPOS_t     chMuxPos[4];
    ADC_CH_MUXINT_t     chMuxInternal[4];
    ADC_CH_MUXNEG_t     chMuxNeg[4];
    ADC_CH_INTMODE_t    chInteruptMode[4];
    ADC_CH_INTLVL_t     chInteruptLvl[4];

    ADC(ADC_t*   pADC);
    ~ADC();
    
    void begin();
    void end();
    
    //! Get and set the DMA values
    /*!
        
    */
    void dma(uint8_t dmasel);
    uint8_t dma();
    
    //! Start an ADC Read manually
    /*!
        \param[in] stMask Bit mask to start ADC channels 0-3
        
        The lower 4 bits of the input parameter are written to the
        Control A register START. The bits that are set will start
        an ADC read on those channels.
    */
    void Start(uint8_t stMask);
    
    virtual void ch0_int() {};
    virtual void ch1_int() {};
    virtual void ch2_int() {};
    virtual void ch3_int() {};
protected:
};

#endif

