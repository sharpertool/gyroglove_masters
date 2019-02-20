#ifndef GYROCMDPROCESSOR_H
#define GYROCMDPROCESSOR_H

#include <avr/io.h>
#include "HardwareSerial.h"
#include "I2C_Master.h"
#include "IMUManager.h"
#include "CmdProcessor.h"

class GyroCmdProcessor : public CmdProcessor
{
    I2C_Master*     _pMaster[4];
    IMUManager*     _pMgr;
    
public:
    GyroCmdProcessor(
        HardwareSerial* pHW,
        I2C_Master**    pMasterList,
        IMUManager*     pMgr
        );
    ~GyroCmdProcessor();
    
    
    void Loop();
    
};

#endif
