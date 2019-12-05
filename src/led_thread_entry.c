#include "led_thread.h"
#include "common.h"

uint8_t led_mode = CONSTANT_OFF;
extern bsp_leds_t leds;

/* LED Thread entry function */
void led_thread_entry(void)
{
    while (1)
    {
        switch(led_mode)
        {
            case CONSTANT_OFF:
            {
                g_ioport.p_api->pinWrite(leds.p_leds[0], IOPORT_LEVEL_HIGH);
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND)/10);
                break;
            }
            case CONSTANT_ON:
            {
                g_ioport.p_api->pinWrite(leds.p_leds[0], IOPORT_LEVEL_LOW);
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND)/10);
                break;
            }

            case LED_SYSTEM_INIT:
            {
                g_ioport.p_api->pinWrite(leds.p_leds[0], IOPORT_LEVEL_LOW);
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND)/5);
                g_ioport.p_api->pinWrite(leds.p_leds[0], IOPORT_LEVEL_HIGH);
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND)/5);
                break;
            }

            case ONLINE_IDLE:
            {
                g_ioport.p_api->pinWrite(leds.p_leds[0], IOPORT_LEVEL_LOW);
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND)/2);
                g_ioport.p_api->pinWrite(leds.p_leds[0], IOPORT_LEVEL_HIGH);
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND)*5);
                break;
            }

            default:
            {
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND)/10);
                break;
            }

        }
    }
}
