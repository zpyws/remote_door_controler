//created by yangwensen@20191107
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "relay.h"
//****************************************************************************************************
#define RLY_PIN     				GET_PIN(A, 7)

#define DOOR_OPEN_DELAY				(4*RT_TICK_PER_SECOND)
//****************************************************************************************************
static struct rt_timer relay_timer;
//****************************************************************************************************
//by yangwensen@20191121
static void relay_timer_timeout(void* parameter)
{
	rt_kprintf("[Y]relay_timer_timeout\r\n");
	relay(0);
}
//****************************************************************************************************
//by yangwensen@20191107
void rt_hw_relay_init(void)
{
	rt_pin_mode(RLY_PIN, PIN_MODE_OUTPUT);
	relay(0);

	rt_timer_init(&relay_timer, "RLY-TMR", relay_timer_timeout, RT_NULL, DOOR_OPEN_DELAY, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
}
//****************************************************************************************************
//by yangwensen@20191107
void relay(rt_uint32_t on)
{
    if(on)rt_pin_write(RLY_PIN, PIN_HIGH);
	else rt_pin_write(RLY_PIN, PIN_LOW);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(relay, [Y]relay on[1] or off[0].)
#endif
//****************************************************************************************************
//by yangwensen@20191121
extern void door_open(void)
{
	relay(1);
	rt_timer_start(&relay_timer);
}
//****************************************************************************************************

