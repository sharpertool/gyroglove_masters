#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "HardwareSerial.h"
#include "ADC.h"

extern HardwareSerial* pdbgserial;

// Generate all of the ISR handlers.. hook them up to a class if/when a class
// is instantiated for a particular USART.
#define ADC_ISR_DEF(periph) \
static ADC*  periph##_master_cp = 0;\
ISR(periph##_CH0_vect) {\
    if (periph##_master_cp) periph##_master_cp->ch0_int();\
} \
ISR(periph##_CH1_vect) {\
    if (periph##_master_cp) periph##_master_cp->ch1_int();\
} \
ISR(periph##_CH2_vect) {\
    if (periph##_master_cp) periph##_master_cp->ch2_int();\
} \
ISR(periph##_CH3_vect) {\
    if (periph##_master_cp) periph##_master_cp->ch3_int();\
}


ADC_ISR_DEF(ADCA);
ADC_ISR_DEF(ADCB);

static void SetADCPointer(ADC_t* ptype, ADC* pm)
{
    if(ptype == &ADCA) {
        ADCA_master_cp = pm;
    } else if (ptype ==  &ADCB) {
        ADCB_master_cp = pm;
    }
}

ADC::ADC(ADC_t* pADC)
{
    _pADC = pADC;
    _dmasel     = 0; // Default no DMA
    _bSigned    = false;
    _bFreeRun   = false;
    
    for (int x=0;x<4;x++) {
        chGain[x]          = ADC_CH_GAIN_1X_gc;
        chInput[x]         = ADC_CH_INPUTMODE_SINGLEENDED_gc;
        chMuxPos[x]        = ADC_CH_MUXPOS_PIN0_gc;
        chMuxNeg[x]        = ADC_CH_MUXNEG_PIN0_gc;
        chMuxInternal[x]   = ADC_CH_MUXINT_DAC_gc;
        chInteruptMode[x]  = ADC_CH_INTMODE_COMPLETE_gc;
        chInteruptLvl[x]   = ADC_CH_INTLVL_LO_gc;        
    }
    SetADCPointer(_pADC,this);
}

ADC::~ADC()
{
    end();
}

//! Initialize the ADC
/*!
    Initialize the ADC register. Most of the parameters are configurable.
    A few are hard-coded, such as the 12Bit mode is fixed. All other parameters
    must be set before calling begin.
    
    The ADC Refernce is fixed to the AREFA. This is in deference to the CFA
    controller, the first project to use this code. In future versions, this
    will need to be set explicitly.
    
    The Event control is set to none. If events are desired, they need to be
    configured seperately.
    
    The prescaler is fixed to divide by 64. I believe this is from the 128Mhz
    clock, which would then yield a 2Mhz ADC Clock. 
    
*/
void ADC::begin()
{
    _pADC->CTRLB =
            (_bSigned ? ADC_CONMODE_bm : 0)
        |   (_bFreeRun ? ADC_FREERUN_bm : 0)
        |   ADC_RESOLUTION_12BIT_gc;

    _pADC->REFCTRL = 
        ADC_REFSEL_AREFA_gc;  
        
    _pADC->EVCTRL = ADC_SWEEP_0123_gc; 
    _pADC->PRESCALER = ADC_PRESCALER_DIV128_gc;
    
    for (int x=0;x<4;x++) {
        ADC_CH_t* pCh = &(_pADC->CH0) + x;
        pCh->CTRL = 
                chGain[x]
            |   chInput[x]
            ;
         
        if (chInput[x] == ADC_CH_INPUTMODE_INTERNAL_gc ) {
            pCh->MUXCTRL = 
                    chMuxInternal[x]
                |   chMuxNeg[x]
                ;
        } else {
            pCh->MUXCTRL = 
                    chMuxPos[x]
                |   chMuxNeg[x]
                ;
        }
            
        pCh->INTCTRL = 
                chInteruptMode[x] << ADC_CH_INTMODE_gp
            |   chInteruptLvl[x] << ADC_CH_INTLVL_gp
            ;
    }
    _pADC->INTFLAGS = 0; // Not using the interupts
        
    _pADC->CTRLA = 
        (ADC_DMASEL_gm & (_dmasel << ADC_DMASEL0_bp))
        | ADC_FLUSH_bm | ADC_ENABLE_bm;
    
}

void ADC::end()
{
    _pADC->CTRLA = 0; // Disable the ADC
    SetADCPointer(_pADC,0);
}

void ADC::dma(uint8_t dmasel)
{
    _dmasel = dmasel;
}

uint8_t ADC::dma()
{
    return _dmasel;
}

void ADC::Start(uint8_t stMask)
{
    _pADC->CTRLA = 
        (ADC_DMASEL_gm & (_dmasel << ADC_DMASEL0_bp)) 
        | ((stMask & 0xf) << ADC_CH0START_bp)
        | ADC_FLUSH_bm | ADC_ENABLE_bm;
}

