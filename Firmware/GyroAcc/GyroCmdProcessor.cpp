
#include <string.h>
#include "GyroCmdProcessor.h"

static char buffer[1024];

extern HardwareSerial* pdbgserial;
extern DebugPort*      pdbgport;

GyroCmdProcessor::GyroCmdProcessor(
        HardwareSerial*     pHW,
        I2C_Master**        pMasterList,
        IMUManager*         pMgr
    ) : CmdProcessor(pHW)
{
    for (int x=0;x<4;x++) {
        _pMaster[x] = (*pMasterList+x);
    }
    _pMgr = pMgr;
}

GyroCmdProcessor::~GyroCmdProcessor()
{
}

void GyroCmdProcessor::Loop()
{
    // Process commands from the command interface.
    //if (false) {
    if (checkCommands()) {
        // Process the command
        
        const char *pCmd = getCmd();

        // What is the best way? An enum would work, but hard to
        // manage. A string is easy, but inefficient... but easy.
        if (strcmp(pCmd,"checkid") == 0) {
            if (paramCnt() < 2) {
                _pHW->print("Fail:incorrect param count\n");
            } else {
                uint8_t idx = 0;
                uint8_t id;
                getParam(0,idx);
                getParam(1,id);
                if (idx < 4) {
                    int retc = _pMaster[idx]->CheckID(id);
                    if (retc == 0) {
                        _pHW->print("Ok:Ack.\n");
                    } else {
                        sprintf(buffer,"Ok:(%d) NAck\n", retc);
                        _pHW->print(buffer);
                    }
                }
            }
        } else if(strcmp(pCmd,"checkids") == 0) {
            int idx = -1;
            if (paramCnt() > 0) {
                getParam(0,idx);
            }
            _pMgr->CheckIDs(_pHW,idx);
            _pHW->print("Ok\n");
        } else if(strcmp(pCmd,"i2cwr") == 0) {
            if (paramCnt() < 4) {
                _pHW->print("Fail:incorrect param count\n");
            } else {
                uint8_t idx = 0;
                uint8_t  ID;
                getParam(0,idx);
                getParam(1,ID);
                uint8_t* pParams;
                pParams = (uint8_t*)malloc(paramCnt());
                for (int x=2;x<paramCnt();x++) {
                    uint8_t p;
                    getParam(x,p);
                    pParams[x-2] = p;
                }
                int retc = _pMaster[idx]->WriteSync(ID, pParams, paramCnt()-2);
                if (retc < 0) {
                    _pHW->println("Fail");
                } else {
                    _pHW->println("Ok");
                }
                free(pParams);
            }
        } else if(strcmp(pCmd,"i2crd") == 0) {
            if (paramCnt() < 4) {
                _pHW->print("Fail:incorrect param count\n");
            } else {
                uint8_t idx = 0;
                uint8_t  ID;
                uint8_t  nRdBytes;
                uint8_t  addr;
                getParam(0,idx);
                getParam(1,ID);
                getParam(2,addr);
                getParam(3,nRdBytes);
                int retc = _pMaster[idx]->WriteReadSync(ID, &addr, 1,nRdBytes);
                if (retc < 0) {
                    sprintf(buffer,"Fail:%d\n",retc);
                    _pHW->print(buffer);
                } else {
                    char* cp = &buffer[0];
                    int n = sprintf(cp,"Ok:");
                    cp += n;
                    for (int x=0;x<_pMaster[idx]->nReadBytes();x++) {
                        int n = sprintf(cp,"0x%x ",_pMaster[idx]->ReadData(x));
                        cp += n;
                    }
                    _pHW->println(buffer);
                }
            }
        } else if(strcmp(pCmd,"streamstart") == 0) {
            uint16_t bUseGyro = 0;
            if (paramCnt() > 0) {
                getParam(0,bUseGyro);
            }
            int retc = _pMgr->StreamStart(bUseGyro == 1);
            if (retc < 0) {
                sprintf(buffer,"Fail:%d\n",retc);
                _pHW->print(buffer);
            } else {
                _pHW->print("Ok\n");
            }
        } else if(strcmp(pCmd,"streamstop") == 0) {
            _pMgr->Stop();
            _pHW->print("Ok\n");
        } else if(strcmp(pCmd,"wd") == 0) {
            // Tell the Stream Manager we are still here
            // Automatically stops if we don't get this after
            // 20 streams..
            _pMgr->StreamWatchdog();
        } else if(strcmp(pCmd,"dbgport") == 0) {
            uint8_t st;
            getParam(0,st);
            pdbgport->SetState(st);
            _pHW->print("Ok\n");
        } else if(strcmp(pCmd,"force_startstop") == 0) {
            _pMgr->ForceStartStop();
            sprintf(buffer,"Ok\n");
            _pHW->print(buffer);
        } else if(strcmp(pCmd,"wiggle") == 0) {
            uint8_t  idx;
            uint16_t cnt;
            uint8_t pinA;
            uint8_t pinB;
            if (paramCnt() < 4) {
                _pHW->print("Fail:Invalid Param Count\n");
            } else {
                getParam(0,idx);
                getParam(1,cnt);
                getParam(2,pinA);
                getParam(3,pinB);
                _pMaster[idx]->WigglePin(cnt,pinA, pinB);
                _pHW->print("Ok\n");
            }
        } else if(strcmp(pCmd,"reset") == 0) {
            _pMgr->Reset();
            sprintf(buffer,"Ok\n");
            _pHW->print(buffer);
        } else if(strcmp(pCmd,"devreset") == 0) {
            _pMgr->ResetDevices();
            sprintf(buffer,"Ok\n");
            _pHW->print(buffer);
        } else if(strcmp(pCmd,"rate") == 0) {
            uint16_t rate;
            getParam(0,rate);
            _pMgr->SampleRate(rate);
            sprintf(buffer,"Ok:%d\n",rate);
            _pHW->print(buffer);
        } else if(strcmp(pCmd,"settime") == 0) {
            uint16_t tval;
            getParam(0,tval);
            sprintf(buffer,"Ok\n");
            _pHW->print(buffer);
        } else if(strcmp(pCmd,"gettime") == 0) {
            sprintf(buffer,"Ok:%d\n",0);
            _pHW->print(buffer);
        } else if(strcmp(pCmd,"baudquery") == 0) {
            sprintf(buffer,"Ok:0x%x 0x%x\n",
                USARTD0.BAUDCTRLA,USARTD0.BAUDCTRLB);
            _pHW->print(buffer);
        } else if(strcmp(pCmd,"debug") == 0) {
            uint16_t tf;
            getParam(0,tf);
            if (tf) {
                pdbgserial->enable(true);
            } else {
                pdbgserial->enable(false);
            }
            sprintf(buffer,"Ok:\n");
            _pHW->print(buffer);
        } else if(strcmp(pCmd,"fast") == 0) {
            // Try and switch the command interface into fast mode.. 
            // see if this actually works!!
            uint16_t tf;
            getParam(0,tf);
            if (tf) {
                _pHW->end();
                _pHW->begin(489795);
            } else {
                _pHW->end();
                _pHW->begin(115200);
            }
        } else {
            sprintf(buffer,"Fail:This is an Invalid Cmd:%s\n",pCmd);
            _pHW->print(buffer);
        }

        resetCmd();
    }
}


