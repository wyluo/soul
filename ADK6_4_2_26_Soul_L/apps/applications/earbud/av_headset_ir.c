#ifdef IR_ENABLE
#include <panic.h>
#include <print.h>
#include <pio.h>
#include <pio_common.h>
#include <system_clock.h>
#include "av_headset.h"
#include "av_headset_ir.h"


#define PIO2BANK(pio) ((uint16)((pio) / 32))
#define PIO2MASK(pio) (1UL << ((pio) % 32))

static void delay_ms(uint16 v_msec_16)
{
    /** add one to the delay to make sure we don't return early */
    uint32 v_delay = SystemClockGetTimerTime() + (v_msec_16 * 1000) + 1;

    while (((int32)(SystemClockGetTimerTime() - v_delay)) < 0);
}

void IREnable(void)
{
    uint16 bank;
    uint32 mask;

    bank = PIO2BANK(IR_EN);
    mask = PIO2MASK(IR_EN);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));
    PanicNotZero(PioSet32Bank(bank, mask, 0));
    PioCommonSetPio(IR_EN,1,FALSE);
    delay_ms(1);

    bank = PIO2BANK(IR_OUT);
    mask = PIO2MASK(IR_OUT);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));
    PanicNotZero(PioSet32Bank(bank, mask, 0));

    delay_ms(1);
    bank = PIO2BANK(IR_GNG);
    mask = PIO2MASK(IR_GNG);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));
    PanicNotZero(PioSet32Bank(bank, mask, mask));

}

void IRDisable(void)
{
    uint16 bank;
    uint32 mask;

    /* Power off the IR*/
    bank = PIO2BANK(IR_EN);
    mask = PIO2MASK(IR_EN);
    PanicNotZero(PioSet32Bank(bank, mask, 1));
}
#endif /*  IR_ENABLE */
