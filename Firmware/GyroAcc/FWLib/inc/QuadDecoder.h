#ifndef QuadDecoder_h
#define QuadDecoder_h

#include <avr/io.h>
#include <inttypes.h>

#include "Print.h"
#include "Port.h"

struct ring_buffer;

class QuadDecoder : public PortNotify
{
private:
    ///#{ Port
    //! Port setup.
    PORT_t          *_port;
    uint8_t         _pPin;
    bool            _binvIO;
    bool            _bUseIndex;
    ///@}
    
    ///@{ Event
    //! Event
    uint8_t         _evMux;
    EVSYS_CHMUX_t   _evPinInput;
    EVSYS_QDIRM_t   _evIndexState;
    ///@}
    
    ///@{ TimerCounter
    //! TimerCounter
    TC0_t*          _pTimer;
    TC_EVSEL_t      _tcEventChannel;
    ///@}
    
    uint16_t        _lineCount;
    bool            _bInverted;
    Port*           _pPort;
    bool            _bUsePort;
    uint16_t        _theCount;
    
public:
    
    typedef enum Direction{
        CW, CCW
    } DirType;
      
    
    QuadDecoder(
        PORT_t          *port,
        uint8_t         portPin,
        bool            bUseIndex,
        bool            bInvertIO,
        
        // Event
        uint8_t         qEvMux,
        EVSYS_CHMUX_t   qPinInput,
        EVSYS_QDIRM_t   qIndexState,
        
        // Timer Counter
        TC0_t*          pTimer,
        TC_EVSEL_t      qEventChannel,
        
        // Hardare definition
        uint16_t        lineCount
        );
    ~QuadDecoder();
    
    bool Setup();
    bool PortSetup();
    bool EventSetup();
    void TC_DecSetup();
    void TC_FreqSetup();
    DirType Get_Direction();
    uint16_t Count();
    void SetCount(uint16_t cnt);
    bool IsInverted();
    void Invert(bool bInv);
    void PinStatus(char& ph0, char& ph90);
    void SetPort(Port* pLeft, uint8_t pin0, uint8_t pin90);
    
    virtual void PortISR0(uint8_t id);
    virtual void PortISR1(uint8_t id);

};


#endif
