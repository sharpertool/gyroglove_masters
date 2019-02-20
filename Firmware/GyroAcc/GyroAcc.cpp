
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "NewDel.h"
#include "clksystem.h"
#include "time.h"

#include "HardwareSerial.h"
#include "GyroCmdProcessor.h"
#include "I2C_Master.h"
#include "IMU.h"
#include "IMUManager.h"
#include "TimerCntr.h"
#include "Port.h"
#include "MyDriver.h"
#include "Revision.h"

//! Declared in TimerCore.cpp
extern void timer_init();

void init() {

    clksystem_init();
    timer_init();
}

void processCmd(CmdProcessor& cmdproc);

HardwareSerial* pdbgserial = 0;
DebugPort*      pdbgport = 0;

int main()
{
    char    buffer[100];
    init();

    PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
    sei();
    
#ifdef USE_DEBUGPORT_2
    DebugPort dbgPort2(&PORTE);
    dbgPort2.PinMask(0xF0,5);
    dbgPort2.SetState(0);
#endif
    
    HardwareSerial dbgserial(&USARTF1, &PORTF, PIN6_bm, PIN7_bm);
    dbgserial.begin(115200);
    pdbgserial = &dbgserial;
    pdbgserial->enable(false);

    // Instantiate a commad processor.
    // Specify the USART, PORT, rxPin and txPin and the baud rate.
    HardwareSerial cmdSerial(&USARTD0, &PORTD, PIN2_bm, PIN3_bm);
    cmdSerial.begin(115200);
    //cmdSerial.begin(285000);
    //cmdSerial.begin(921600);
    
    CCP = CCP_IOREG_gc;
    MCU.MCUCR = MCU_JTAGD_bm;
    DebugPort dbgPort(&PORTB);
    dbgPort.PinMask(0xF0,4);
    dbgPort.SetState(0xf);
    dbgPort.SetState(0);
    pdbgport = &dbgPort;

    I2C_Master  *pMaster[4];
    I2C_Master hand(&TWIC);
    I2C_Master single(&TWID);
    I2C_Master pair1(&TWIE);
    I2C_Master pair2(&TWIF);

    pMaster[0] = &hand;
    pMaster[1] = &single; // Pinkie
    pMaster[2] = &pair1;  // Ring + Middle
    pMaster[3] = &pair2;  // Index + Thumb
    
    for (int x=0;x<4;x++) {
        if (pMaster[x]) {
            pMaster[x]->begin(400e3);
        }
    }

    // Give the hardware time to stabilize..
    _delay_ms(1000);
    
    // When constructed without a list of ID's, the IMU will query known
    // ID's 0xD0 and 0xD2 for the Gyro, and then 0x30 and 0x32 respectively
    // for the ACC.
    IMU     hand_imu(&hand);    
    IMU     single_imu(&single);
    IMU     pair1_imu(&pair1);  
    IMU     pair2_imu(&pair2);  

    // These all share the debug port, so they must operate
    // in sequence. If I went back to a parallel operation, then
    // this would have to change.
    hand_imu.SetDebugPort(&dbgPort);

    single_imu.SetDebugPort(&dbgPort);

    pair1_imu.SetDebugPort(&dbgPort);

    pair2_imu.SetDebugPort(&dbgPort);

#ifdef USE_DEBUGPORT_2
    hand_imu.SetDebugPort2(&dbgPort2);
    single_imu.SetDebugPort2(&dbgPort2);
    pair1_imu.SetDebugPort2(&dbgPort2);
    pair2_imu.SetDebugPort2(&dbgPort2);
#endif
    
    //! Create timers for each of the IMU Masters. I use the TCx1 timers which are
    //! less capable, but it hardly matters as these are just setting a timer callback.
    TimerCntr   tcA(&TCC1);
    
    IMUManager imumgr(&cmdSerial);
    imumgr.SetBlueLed(&PORTJ, PIN7_bm);
    imumgr.LedOff();
    imumgr.SetTimer(&tcA);
    imumgr.AddIMU(&hand_imu);
    imumgr.AddIMU(&single_imu);
    imumgr.AddIMU(&pair1_imu);
    imumgr.AddIMU(&pair2_imu);
    imumgr.SampleRate(10); /// Give the rate a default value. Start low
    
    GyroCmdProcessor cmdproc(&cmdSerial,&pMaster[0],&imumgr);

    sprintf(buffer,"Welcome to Gyro Glove.\nRev %d.%d.%d Date: %02d/%02d/%04d Built at: %02d:%02d\n",
        RevMajor, RevMinor, RevInc,
        DateMonth, DateDay, DateYear,
        TimeHour, TimeMin
        );
    cmdSerial.print(buffer);
    
    //! This bit just toggles the light
    TimerCntr mdtc(&TCD0);
    MyDriver md;
    md.SetTimer(&mdtc);
    
    cmdSerial.print("Starting endless loop\n");
    
    while(1) {
        cmdproc.Loop();        
        imumgr.Loop();
    }

    return 0;
}


