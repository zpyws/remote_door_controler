/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-01-01     aozima       first implementation.
 * 2012-07-27     aozima       fixed variable uninitialized.
 */
#include <board.h>
#include "drv_spi.h"
//#include "spi_flash.h"
#include "spi_flash_sfud.h"


static int rt_hw_spi_flash_with_sfud_init(void)
{
	rt_err_t result;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	result = rt_hw_spi_device_attach("spi1", "spi10", GPIOA, GPIO_PIN_15);
	if (result != RT_EOK)
	{
		return result;
	}

    if (RT_NULL == rt_sfud_flash_probe("W25Q128", "spi10"))
    {
        return RT_ERROR;
    }

	return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_with_sfud_init);
