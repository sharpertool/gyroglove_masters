
#ifndef _imupacket_h
#define _imupacket_h

#include <inttypes.h>

/*! IMU Packet data

Encapsulate the data from an IMU packet into a single object.
*/ 
class IMUPacket
{
public:
    
    uint8_t*    _pData;
    
    static const uint16_t PacketLen = 16;
    static const uint16_t ResetLen = 3000;
    
    IMUPacket() 
    {
        _pData = (uint8_t*)malloc(PacketLen+1);
        *_pData = 0;// Ref count
    }
    
    //! Copy constructor just makes a new copy.
    IMUPacket(const IMUPacket& rPacket) 
    {
        _pData = rPacket._pData;
        (*_pData)++;
    }
        
    IMUPacket(uint8_t* pd)
    {
        _pData = (uint8_t*)malloc(PacketLen+1);
        memcpy(_pData+1,pd,PacketLen);
        *_pData = 1;// Ref count
    }
    
    ~IMUPacket()
    {
        (*_pData)--;
        if (*_pData == 0) {
            free(_pData);
        }
    }
    
    IMUPacket& operator=(const IMUPacket& rhs)
    {
        memcpy(_pData+1,rhs._pData+1,PacketLen);
        return *this;
    }
    
    //! Fill buffer with Temp, gyro*3 and acc*3
    uint8_t* FillBuffer(uint8_t* pBuffer)
    {
#ifdef DEBUG
        static uint8_t tbuffer[] = {
            0x5a, 
            0x5a,
            0x11,
            0x22,
            0x33,
            0x44,
            0x55,
            0x66,
            0x77,
            0x88,
            0x44,
            0x22,
            0x11,
            0xCC,
            0x00,
            0x00
        };
        
        memcpy(pBuffer,tbuffer,PacketLen);
#else
        memcpy(pBuffer,_pData+1,PacketLen);
#endif
        return pBuffer+PacketLen;
    }
    
};

#endif


