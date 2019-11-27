/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   change to new framework
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "door.h"
#include "led.h"

int main(void)
{
	door_init();

    while (1)
    {
        rt_thread_mdelay(500);
		led(0,2);
    }

    return RT_EOK;
}
