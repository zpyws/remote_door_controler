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
#include <dfs_fs.h>

int main(void)
{
    if (dfs_mount("W25Q128", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("spi flash mount to / !\n");
    }
    else
    {
        rt_kprintf("spi flash mount to / failed!\n");
    }

	door_init();

    while (1)
    {
        rt_thread_mdelay(500);
		rt_hw_led_tog(0);
    }
}
