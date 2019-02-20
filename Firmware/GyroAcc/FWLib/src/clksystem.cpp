#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


/*! \brief This macro will protect the following code from interrupts. */
#define AVR_ENTER_CRITICAL_REGION( ) uint8_t volatile saved_sreg = SREG; \
                                     cli();

/*! \brief This macro must always be used in conjunction with AVR_ENTER_CRITICAL_REGION
 *        so the interrupts are enabled again.
 */
#define AVR_LEAVE_CRITICAL_REGION( ) SREG = saved_sreg;

/*! \brief CCP write helper function written in assembly.
 *
 *  This function is written in assembly because of the timecritial
 *  operation of writing to the registers.
 *
 *  \param address A pointer to the address to write to.
 *  \param value   The value to put in to the register.
 */
void CCPWrite( volatile uint8_t * address, uint8_t value )
{
    AVR_ENTER_CRITICAL_REGION( );

    volatile uint8_t * tmpAddr = address;

    asm volatile(
        "movw r30,  %0"	      "\n\t"
        "ldi  r16,  %2"	      "\n\t"
        "out   %3, r16"	      "\n\t"
        "st     Z,  %1"       "\n\t"
        :
        : "r" (tmpAddr), "r" (value), "M" (0xD8), "i" (&CCP)
        : "r16", "r30", "r31"
    );

    AVR_LEAVE_CRITICAL_REGION( );
}

/**********************************
Function: init
Purpose:
A: Setup the clocking system.
B: To Be Determined

We have an 8Mhz external oscillator. Set the PLL to multiply by
16, to give a peripheral clock of 128Mhz.
Set prescaler A to be 1, to get a 128Mhz peripheral clock.
Set prescaler B to be 2 to get a 64Mhz 2X speed clock
Set presclaer C to be 2 to get a 32Mhz CPU Clock

**********************************/
void clksystem_init()
{
    // Disable first, then set the XOSCTRL, then enable.
    OSC.CTRL &= ~OSC_XOSCEN_bm;
    OSC.XOSCCTRL = OSC_FRQRANGE_9TO12_gc | OSC_XOSCSEL_XTAL_16KCLK_gc; // Using an external xtal
    OSC.CTRL |= OSC_XOSCEN_bm;

    // Wait for the external oscilator. Don't wait forever though.
    int maxWait = 100; // Wait 1 second
    while (--maxWait && !(OSC.STATUS & OSC_XOSCRDY_bm)) {
        _delay_ms(10);
    }


    // If the external oscilator is running, then we can switch the PLL
    // over to it and wait for the PLL to stabilize.
    if (OSC.STATUS & OSC_XOSCRDY_bm) {
        // Setup the PLL to use the external 8Mhz oscillator and a
        // factor of 16 to get a 128Mhz PLL Clock.

        // Make sure this is disabled before we try and configure it.
        OSC.CTRL &= ~OSC_PLLEN_bm;

        OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | (16 << OSC_PLLFAC_gp);

        OSC.CTRL |= OSC_PLLEN_bm;

        //OSC.CTRL = CLK_PSADIV_16_gc;
        // Wait for OSC.STATUS to indicate that PLL is ready..
        while (!(OSC.STATUS & OSC_PLLRDY_bm)) {
            _delay_ms(10);
        }

        // Okay, the PLL is up on the new clock. Set the prescalers then
        // switch over to the PLL.

        CCP = CCP_IOREG_gc;
        CLK.PSCTRL = CLK_PSADIV_1_gc | CLK_PSBCDIV_2_2_gc;

        // When all is done, make the PLL be the clock source for the system.
        CCP = CCP_IOREG_gc;
        CLK.CTRL = CLK_SCLKSEL_PLL_gc;
    }
}
