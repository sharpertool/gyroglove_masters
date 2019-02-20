#ifndef MARK_h
#define MARK_h

class Mark
{
    DebugPort* _pDbgPort;
public:
    Mark(DebugPort* dbgpport, uint8_t p) 
    {
        _pDbgPort = dbgpport;
        if (_pDbgPort) {
            _pDbgPort->SetState((uint8_t) 1);
            _delay_us(3);
            _pDbgPort->SetState((uint8_t) p);
        }
    }
    
    ~Mark() {
        _pDbgPort->SetState((uint8_t) 0);
    }
};
        
#endif
