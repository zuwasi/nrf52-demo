#include <stdint.h>

extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

typedef void (*isr_handler_t)(void);

__attribute__((section(".isr_vector")))
const isr_handler_t vector_table[] =
{
    (isr_handler_t)&_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    SVC_Handler,
    Default_Handler,
    Default_Handler,
    PendSV_Handler,
    SysTick_Handler
};

void Reset_Handler(void)
{
    uint32_t * source = &_sidata;
    uint32_t * destination = &_sdata;

    while (destination < &_edata)
    {
        *destination = *source;
        destination++;
        source++;
    }

    destination = &_sbss;

    while (destination < &_ebss)
    {
        *destination = 0U;
        destination++;
    }

    (void)main();

    for (;;)
    {
        /* main should not return. */
    }
}

void Default_Handler(void)
{
    for (;;)
    {
        /* Fault/unused interrupt trap. */
    }
}
