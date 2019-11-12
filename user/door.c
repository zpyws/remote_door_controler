//created by yangwensen@20191112
#include <rtthread.h>
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
static char txbuff[80];

door_info_t door_info;
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
extern void door_init(void)
{
	rt_memset(&door_info, 0, sizeof(door_info));
	
	rt_memcpy(door_info.IMEI, "863412045887166", sizeof(door_info.IMEI)-1);
	rt_memcpy(door_info.ICCID, "898602B6101680792546", sizeof(door_info.ICCID)-1);
	rt_memcpy(door_info.auth_code, "Kijni1nld", sizeof(door_info.auth_code)-1);
}
//************************************************************************************************************
extern int8_t door_register(void)
{
	uint8_t len;
	
	len = rt_sprintf(txbuff, "R:%04X:%s:%s:%s:%s\n", door_info.session_id, door_info.IMEI, door_info.ICCID, door_info.auth_code, "898604241118C0010270");
	DOOR_WRITE(txbuff, len);
	
	return 0;
}
//************************************************************************************************************
