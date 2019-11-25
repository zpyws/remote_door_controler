//created by yangwensen@20191107
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "led.h"
//****************************************************************************************************
#define LED1_PIN     				GET_PIN(B, 0)
#define LED2_PIN     				GET_PIN(A, 8)
#define LED3_PIN     				GET_PIN(C, 6)
//****************************************************************************************************
//by yangwensen@20181022
static int rt_hw_led_init(void)
{
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED3_PIN, PIN_MODE_OUTPUT);
	
	rt_hw_led_off(1);
	rt_hw_led_off(2);
	rt_hw_led_off(3);

	return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_led_init);

void rt_hw_led_on(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        rt_pin_write(LED1_PIN, PIN_HIGH);
        break;
    case 1:
        rt_pin_write(LED2_PIN, PIN_HIGH);
        break;
    case 2:
        rt_pin_write(LED3_PIN, PIN_HIGH);
        break;
    default:
        break;
    }
}

void rt_hw_led_off(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        rt_pin_write(LED1_PIN, PIN_LOW);
        break;
    case 1:
        rt_pin_write(LED2_PIN, PIN_LOW);
        break;
    case 2:
        rt_pin_write(LED3_PIN, PIN_LOW);
        break;
    default:
        break;
    }
}

//by yangwensen@20181012
void rt_hw_led_tog(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        rt_pin_write(LED1_PIN, !rt_pin_read(LED1_PIN));
        break;
    case 1:
        rt_pin_write(LED2_PIN, !rt_pin_read(LED2_PIN));
        break;
    case 2:
        rt_pin_write(LED3_PIN, !rt_pin_read(LED3_PIN));
        break;
    default:
        break;
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static rt_uint8_t led_inited = 0;
void led(rt_uint32_t led, rt_uint32_t value)
{
    /* init led configuration if it's not inited. */
    if (!led_inited)
    {
        rt_hw_led_init();
        led_inited = 1;
    }

	switch (value)
	{
	case 0:
		rt_hw_led_off(led);
		break;
	case 1:
		rt_hw_led_on(led);
		break;
	case 2:
		rt_hw_led_tog(led);
		break;
	default:
		break;
	}
}
FINSH_FUNCTION_EXPORT(led, [Y]set led[1-3] on[1] or off[0].)
#endif

