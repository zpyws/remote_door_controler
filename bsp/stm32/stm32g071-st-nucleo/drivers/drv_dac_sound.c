/*
 * File: drv_sound.c
 * 
 * COPYRIGHT (C) 2012-2019, Shanghai Real-Thread Technology Co., Ltd
 */

#include "drv_dac_sound.h" 
//#include "drv_tina.h" 
#include "drivers/audio.h"

#define DBG_TAG "drv_sound"
#define DBG_LVL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#define TX_DMA_FIFO_SIZE (2048)

struct temp_sound
{
    struct rt_audio_device device; 
    struct rt_audio_configure replay_config;
    int volume;
    rt_uint8_t *tx_fifo;
};

static rt_err_t getcaps(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
    struct temp_sound *sound = RT_NULL;

    RT_ASSERT(audio != RT_NULL); 
    sound = (struct temp_sound *)audio->parent.user_data;

    return RT_EOK; 
}

static rt_err_t configure(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
    struct temp_sound *sound = RT_NULL;

    RT_ASSERT(audio != RT_NULL); 
    sound = (struct temp_sound *)audio->parent.user_data;

    return RT_EOK; 
}

static rt_err_t init(struct rt_audio_device *audio)
{
    struct temp_sound *sound = RT_NULL;

    RT_ASSERT(audio != RT_NULL); 
    sound = (struct temp_sound *)audio->parent.user_data;

    return RT_EOK; 
}

static rt_err_t start(struct rt_audio_device *audio, int stream)
{
    struct temp_sound *sound = RT_NULL;

    RT_ASSERT(audio != RT_NULL); 
    sound = (struct temp_sound *)audio->parent.user_data;

    return RT_EOK;
}

static rt_err_t stop(struct rt_audio_device *audio, int stream)
{
    struct temp_sound *sound = RT_NULL;

    RT_ASSERT(audio != RT_NULL); 
    sound = (struct temp_sound *)audio->parent.user_data;    

    return RT_EOK;
}

rt_size_t transmit(struct rt_audio_device *audio, const void *writeBuf, void *readBuf, rt_size_t size)
{
    struct temp_sound *sound = RT_NULL;

    RT_ASSERT(audio != RT_NULL); 
    sound = (struct temp_sound *)audio->parent.user_data;

    return size; 
}

static void buffer_info(struct rt_audio_device *audio, struct rt_audio_buf_info *info)
{
    struct temp_sound *sound = RT_NULL;

    RT_ASSERT(audio != RT_NULL); 
    sound = (struct temp_sound *)audio->parent.user_data;

    /**
     *               TX_FIFO
     * +----------------+----------------+
     * |     block1     |     block2     |
     * +----------------+----------------+
     *  \  block_size  /
     */
    info->buffer      = sound->tx_fifo;
    info->total_size  = TX_DMA_FIFO_SIZE;
    info->block_size  = TX_DMA_FIFO_SIZE / 2;
    info->block_count = 2;
}

static struct rt_audio_ops ops =
{
    .getcaps     = getcaps,
    .configure   = configure,
    .init        = init,
    .start       = start,
    .stop        = stop,
    .transmit    = transmit, 
    .buffer_info = buffer_info,
};

static int rt_hw_sound_init(void)
{
    rt_uint8_t *tx_fifo = RT_NULL; 
    static struct temp_sound sound = {0};

    /* 分配 DMA 搬运 buffer */ 
    tx_fifo = rt_calloc(1, TX_DMA_FIFO_SIZE); 
    if(tx_fifo == RT_NULL)
    {
        return -RT_ENOMEM;
    }

    sound.tx_fifo = tx_fifo;

    /* 注册声卡放音驱动 */
    sound.device.ops = &ops;
    rt_audio_register(&sound.device, "sound0", RT_DEVICE_FLAG_WRONLY, &sound);

    return RT_EOK; 
}
INIT_DEVICE_EXPORT(rt_hw_sound_init);
