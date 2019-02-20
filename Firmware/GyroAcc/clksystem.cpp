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
    // This board does not have an external oscillator,
    // so we need to use the internal oscillator.
    // Enable the internal 32Mhz oscillator. Disables all the rest.
    OSC.CTRL = OSC_RC32MEN_bm | OSC_RC2MEN_bm | OSC_RC32KEN_bm;

    //OSC.XOSCCTRL = 0; // we don't care, just set this to some value.

    // Wait for the external oscilator. Don't wait forever though.
    int maxWait = 100; // Wait 1 second
    while (--maxWait && !(OSC.STATUS & (OSC_RC32MRDY_bm | OSC_RC32KRDY_bm | OSC_RC2MRDY_bm))) {
        _delay_loop_2(10);
    }

    OSC.DFLLCTRL = 0;
    DFLLRC32M.CTRL = 0x1;
    DFLLRC2M.CTRL = 0x1;

    // If the external oscilator is running, then we can switch the PLL
    // over to it and wait for the PLL to stabilize.
    if (OSC.STATUS & ( OSC_RC32MRDY_bm | OSC_RC2MRDY_bm)) {
        // Setup the PLL to use the internal 32Mhz oscillator and a
        // factor of 4 to get a 128Mhz PLL Clock.

        // Make sure this is disabled before we try and configure it.
        OSC.CTRL &= ~OSC_PLLEN_bm;

        OSC.PLLCTRL = OSC_PLLSRC_RC32M_gc | (16 << OSC_PLLFAC_gp);

        OSC.CTRL |= OSC_PLLEN_bm;

        //OSC.CTRL = CLK_PSADIV_16_gc;
        // Wait for OSC.STATUS to indicate that PLL is ready..
        while (!(OSC.STATUS & OSC_PLLRDY_bm)) {
        }

        // Okay, the PLL is up on the new clock. Set the prescalers then
        // switch over to the PLL.

        CCP = CCP_IOREG_gc;
        CLK.PSCTRL = CLK_PSADIV_1_gc | CLK_PSBCDIV_2_2_gc;
        //CLK.PSCTRL = CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc;

        // When all is done, make the PLL be the clock source for the system.
        CCP = CCP_IOREG_gc;
        CLK.CTRL = CLK_SCLKSEL_PLL_gc;
        //CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
        //CLK.CTRL = CLK_SCLKSEL_RC2M_gc;
    }
}
