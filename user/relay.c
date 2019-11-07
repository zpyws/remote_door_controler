//created by yangwensen@20191107
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "relay.h"
//****************************************************************************************************
#define RLY_PIN     				GET_PIN(A, 7)
//****************************************************************************************************
//by yangwensen@20191107
void rt_hw_relay_init(void)
{
	rt_pin_mode(RLY_PIN, PIN_MODE_OUTPUT);
	relay(0);
}

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

