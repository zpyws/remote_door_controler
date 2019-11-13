//created by yangwensen@20191112
#include <rtthread.h>
#include <at.h>
#include <rtdbg.h>
//************************************************************************************************************
//by yangwensen@20191112
typedef struct
{
	uint8_t IMEI[15+1];
	uint8_t ICCID[20+1];
	
	uint8_t auth_code[9+1];
	uint16_t session_id;
}door_info_t;
//************************************************************************************************************
static void urc_func(const char *data, rt_size_t size);
static void create_door_server_process(void);
int door_client_obj_init(void);
//************************************************************************************************************
static char txbuff[80];

door_info_t door_info;

//by yangwensen@20191112
static struct at_urc door_urc_table[] = 
{
	{"OK:",		"\n", 	urc_func},
	{"ERR:",	"\n", 	urc_func},
	{"O:",		"\n", 	urc_func},
	{"C:",		"\n", 	urc_func},
	{"T:",		"\n", 	urc_func},
	{"IP:",		"\n", 	urc_func},
	{"S:",		"\n", 	urc_func},
	{"U:",		"\n", 	urc_func},
};
//************************************************************************************************************
#define DOOR_WRITE(a,b)		memdump((uint8_t *)(a),b)
//************************************************************************************************************
//by yangwensen@20191112
extern void memdump(uint8_t *buff, uint16_t len)
{
	uint16_t i;
	
	rt_kprintf("[%d]\r\n", len);
	for(i=0; i<len; i++)
	{
		rt_kprintf("%02X ", buff[i]);
		if(i && (i&15)==15)rt_kprintf("\r\n");
	}
	rt_kprintf("\r\n");
}
//************************************************************************************************************
//by yangwensen@20191112
static void urc_func(const char *data, rt_size_t size)
{
    LOG_D("URC data : %.*s", size, data);
}
//************************************************************************************************************
//by yangwensen@20191112
extern void door_init(void)
{
	rt_memset(&door_info, 0, sizeof(door_info));
	
	rt_memcpy(door_info.IMEI, "863412045887166", sizeof(door_info.IMEI)-1);
	rt_memcpy(door_info.ICCID, "898602B6101680792546", sizeof(door_info.ICCID)-1);
	rt_memcpy(door_info.auth_code, "Kijni1nld", sizeof(door_info.auth_code)-1);
	
//	door_client_obj_init();
	create_door_server_process();
}
//************************************************************************************************************
//by yangwensen@20191112
extern int8_t door_register(void)
{
	uint8_t len;
	
	len = rt_sprintf(txbuff, "R:%04X:%s:%s:%s:%s\n", door_info.session_id, door_info.IMEI, door_info.ICCID, door_info.auth_code, "898604241118C0010270");
	DOOR_WRITE(txbuff, len);
	
	return 0;
}
//************************************************************************************************************
//by yangwensen@20191112
extern int8_t door_heart_beat(void)
{
	uint8_t len;
	int8_t error = 0;
	
	len = rt_sprintf(txbuff, "H:ffff:%d\n", error);
	DOOR_WRITE(txbuff, len);
	
	return 0;
}
//************************************************************************************************************
//by yangwensen@20191112
int door_client_obj_init(void)
{
	at_client_init("ec20", 128);
	at_set_urc_table(door_urc_table, sizeof(door_urc_table) / sizeof(door_urc_table[0]));
	
	return RT_EOK;
}
//************************************************************************************************************
//by yangwensen@20191113
static void task_door_server(void* parameter)
{
	while(1)
	{
        rt_thread_mdelay(1000);	
		rt_kprintf("task_door_server\r\n");
	}
}
//************************************************************************************************************
//by yangwensen@20191113
static void create_door_server_process(void)
{
    rt_thread_t tid;
	
    tid = rt_thread_create("ec20_net_init", task_door_server, RT_NULL, 512, 25, 20);
    if (tid)
    {
        rt_thread_startup(tid);
    }
}
//************************************************************************************************************
