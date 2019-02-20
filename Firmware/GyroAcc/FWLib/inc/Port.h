
#ifndef Port_h
#define Port_h

//! Base class for any classes that require port change notification
//! If a class requires notification of a port change event, then use this
//! class as a base class. Call the Notify method of any Port class and
//! pass in the 'this' pointer of the class to notify. Also pass in an index
//! value. The index value is needed becuase it is possible for a single class
//! to request notifications from 2 or more Ports. In order to know which of
//! the ports is sending the notification, use a unique index for both. The
//! actual value of the index is not important.
class PortNotify
{
public:
    virtual void PortISR0(uint8_t id) = 0;
    virtual void PortISR1(uint8_t id) = 0;
};

//! Class to handle the setup and control of a port on the ATxmega.
//! This class will setup the port using the PinControl method. It will
//! also setup and receive interrupts on either int0 or int1. You may set
//! an interrupt mask to determine which pins cause which interrupt.
//! For example, it is possible to have pins 0 and 1 cause interrupts on 
//! ISR0, and pins 6 and 7 to interrupt on ISR1. 
class Port
{
protected:
    PORT_t*     _pPort;
    PortNotify* _pNotifyClient;
    uint8_t     _pNotifyID;
    
public:

    Port(PORT_t*);
    ~Port();

    void Notify(PortNotify* pClient,uint8_t id);
    void int0();
    void int1();
    
    void SetDir(uint8_t dir);
    void SetPinsAsInput(uint8_t mask);
    void SetPinsAsOutput(uint8_t mask);
    void SetPinsHigh(uint8_t mask);
    void SetPinsLow(uint8_t mask);
    uint8_t GetPins();
    
    //! Interrupt control
    void InterruptLevel(uint8_t num, uint8_t lvl);
    void InterruptMask(uint8_t num, uint8_t mask);
    
    //! Pin Control Register
    void PinControl(uint8_t mask, 
        bool bSlewLimit,
        bool bInverted,
        PORT_OPC_t OutputConfig,
        PORT_ISC_t InputSense
        );
};


#endif
