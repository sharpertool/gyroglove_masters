#ifndef CFADAC_H
#define CFADAC_H

#include <stdlib.h>
#include <avr/io.h>
#include "SPI.h"


class CFADac
{
private:
    SPI*        _pSPI;
    
public:
    CFADac(SPI_t*   pSPI);
    ~CFADac();
    void writeInput(uint8_t n, uint16_t val);
    void powerUp(uint8_t n);
    void writeInputPowerUpAll(uint8_t n, uint16_t val);
    void powerUp(uint8_t n,uint16_t val);
    void powerDown(uint8_t n);

};

#endif

