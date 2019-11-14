//created by yangwensen@20191112
#define DBG_LVL				DBG_LOG
#define DBG_TAG				"door"
//************************************************************************************************************
#include <rtthread.h>
#include <at.h>
#include <rtdbg.h>
#include <sys/socket.h>
#include "door.h"
#include <sys/time.h>
#include <string.h>
//************************************************************************************************************
static void urc_func(const char *data, rt_size_t size);
static void create_door_server_process(void);
int door_client_obj_init(void);
extern int8_t door_register(int sock);
static int8_t check_response(char *str, uint8_t len, uint16_t sesson_id);
static int8_t recv_data_resolve(char *buff, uint32_t len);

//cmd
static void cmd_query_status(const char *data, rt_size_t size);
static void cmd_open_door(const char *data, rt_size_t size);
static void cmd_update_soundcode(const char *data, rt_size_t size);
static void cmd_volume(const char *data, rt_size_t size);
//************************************************************************************************************
#define BUFSZ   1024

door_info_t door_info;

//by yangwensen@20191112
static const struct at_urc door_urc_table[] = 
{
	{"OK:",		"\n", 	urc_func},
	{"ERR:",	"\n", 	urc_func},
	{"Q:",		"\n", 	cmd_query_status},
	{"O:",		"\n", 	cmd_open_door},
	{"S:",		"\n", 	cmd_update_soundcode},
	{"V:",		"\n", 	cmd_volume},
};
//************************************************************************************************************
//#define DOOR_WRITE(a,b)		memdump((uint8_t *)(a),b)
#define DOOR_WRITE(a,b)		tcp_write(sock,(uint8_t *)(a),b)

#define SERVER_IP			"119.3.128.217"
#define SERVER_PORT			8091

#define ARRAY_SIZE(a)		(sizeof(a)/sizeof(a[0]))
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
//by yangwensen@20191113
static int tcp_write(int sock, uint8_t *buff, uint32_t len)
{
	int ret;
	
	ret = send(sock, buff, len, 0);
	if(ret!=len)LOG_E("[Y]TCP send %d bytes of %d\r\n", ret, len);
	else LOG_I("[Y]TCP send %d bytes\r\n", len);
	
	return ret;
}
//************************************************************************************************************
//by yangwensen@20191112
extern void door_init(void)
{
//	rt_memset(&door_info, 0, sizeof(door_info));
	
//	rt_memcpy(door_info.IMEI, "863412045887166", sizeof(door_info.IMEI)-1);
//	rt_memcpy(door_info.ICCID, "898602B6101680792546", sizeof(door_info.ICCID)-1);
	rt_memcpy(door_info.auth_code, "Kijni1nld", sizeof(door_info.auth_code)-1);
	LOG_I("[Y]IMEI: %s", door_info.IMEI);
	memdump(door_info.IMEI, sizeof(door_info.IMEI));
	LOG_I("[Y]ICCID: %s", door_info.ICCID);
	memdump(door_info.ICCID, sizeof(door_info.ICCID));
	
//	door_client_obj_init();
	create_door_server_process();
}
//************************************************************************************************************
//by yangwensen@20191112
static uint8_t door_register_str(char *str)
{
	return rt_sprintf(str, "R:%04X:%s:%s:%s:%s\n", door_info.session_id, door_info.IMEI, door_info.ICCID, door_info.auth_code, "898604241118C0010270");
}
//************************************************************************************************************
//by yangwensen@20191114
extern int8_t door_heart_beat(int sock, char *str)
{
	uint8_t len;
	int8_t error = 0;
	int ret;
	
	len = rt_sprintf(str, "H:%s:%d\n", door_info.IMEI, error);
	ret = tcp_write(sock, (uint8_t *)str, len);
	
	LOG_D("[Y]recv start=0x%08X\r\n", rt_tick_get());
	ret = recv(sock, str, BUFSZ - 1, 0);
	LOG_D("[Y]recv end=0x%08X\r\n", rt_tick_get());
	if(ret<=0)
	{
		LOG_E("[Y]recv error=%d\r\n", ret);
        return -1;
	}

	if( check_response(str, ret, door_info.session_id) != 0)
	{
		LOG_E("[Y]check server response data error\r\n");
        return -2;
	}
	LOG_D("[Y]get heart beat pack response!\r\n");

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
static int tcp_client(char *server_ip, int server_port)
{
	int sock = -1;
	struct sockaddr_in server_addr;
	char *recv_data = RT_NULL;
	int ret,len;
	struct timeval timeout;
	
    recv_data = rt_malloc(BUFSZ);
    if (recv_data == RT_NULL)
    {
        LOG_E("No memory");
        return -1;
    }
	
	sock = socket(AF_AT, SOCK_STREAM, 0);
	if(sock < 0)
	{
		LOG_E("[Y]Create socket error");
		goto __TCP_CLIENT_EXIT;
	}
	LOG_I("[Y]Socket allocated ok\n");
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        LOG_E("[Y]Connect to dingdong home server fail!");
        goto __TCP_CLIENT_EXIT;
    }
	LOG_I("[Y]Connect to dingdong home server OK!");
//=====================================================================================	
	//����ע��
	len = door_register_str(recv_data);
	ret = tcp_write(sock, (uint8_t *)recv_data, len);

	LOG_D("[Y]recv start=0x%08X\r\n", rt_tick_get());
	ret = recv(sock, recv_data, BUFSZ - 1, 0);
	LOG_D("[Y]recv end=0x%08X\r\n", rt_tick_get());
	if(ret<=0)
	{
		LOG_E("[Y]recv error=%d\r\n", ret);
        goto __TCP_CLIENT_EXIT;
	}
	
	if( check_response(recv_data, ret, door_info.session_id) != 0)
	{
		LOG_E("[Y]check server response data error\r\n");
        goto __TCP_CLIENT_EXIT;
	}
	LOG_I("[Y]door access controller registered!\r\n");
//=====================================================================================	
	timeout.tv_sec = 10;
	timeout.tv_usec =  0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
	while(1)
	{
		ret = recv(sock, recv_data, BUFSZ - 1, 0);
		
		if(ret==-1 && errno==EAGAIN)
		{
			LOG_W("[Y]recv timeout,send heart beat pack\r\n");
			door_heart_beat(sock, recv_data);
			continue;
		}
		
		recv_data_resolve(recv_data, ret);
	}
//=====================================================================================	
__TCP_CLIENT_EXIT:	
    if (recv_data)
    {
        rt_free(recv_data);
        recv_data = RT_NULL;
    }
	
	if(sock >=0)
	{
		closesocket(sock);
		sock = -1;
	}
	return -1;
}
//************************************************************************************************************
//by yangwensen@20191113
static void task_door_server(void* parameter)
{
	tcp_client(SERVER_IP, SERVER_PORT);
	
	while(1)
	{
        rt_thread_mdelay(1000);	
//		rt_kprintf("task_door_server\r\n");
	}
}
//************************************************************************************************************
//by yangwensen@20191113
static void create_door_server_process(void)
{
    rt_thread_t tid;
	
    tid = rt_thread_create("tcp_client", task_door_server, RT_NULL, 1024, 25, 20);
    if (tid)
    {
        rt_thread_startup(tid);
    }
}
//************************************************************************************************************
//by yangwensen@20191114
static int8_t check_response(char *str, uint8_t len, uint16_t sesson_id)
{
	const char STR_OK[] = "OK";
	const char STR_ERR[] = "ERR";
	
	str[len] = 0;
	LOG_D("[Y][server]%s\n", str);
	
//	if(str[len-1]!='\n')return -1;
	
	if( rt_memcmp(str, STR_OK, sizeof(STR_OK)-1) == 0 )
	{
		return 0;
	}
	else if( rt_memcmp(str, STR_ERR, sizeof(STR_ERR)-1) == 0 )
	{
		return 1;
	}
	
	return -2;
}
//************************************************************************************************************
//by yangwensen@20191114
static int8_t recv_data_resolve(char *buffer, uint32_t buf_sz)
{
    rt_size_t i, prefix_len, suffix_len;

	memdump((uint8_t *)buffer, buf_sz);
	
    for (i = 0; i < ARRAY_SIZE(door_urc_table); i++)
    {
        prefix_len = strlen(door_urc_table[i].cmd_prefix);
        suffix_len = strlen(door_urc_table[i].cmd_suffix);
        if (buf_sz < prefix_len + suffix_len)
        {
            continue;
        }
		
        if ((prefix_len ? !strncmp(buffer, door_urc_table[i].cmd_prefix, prefix_len) : 1)
                && (suffix_len ? !strncmp(buffer + buf_sz - suffix_len, door_urc_table[i].cmd_suffix, suffix_len) : 1))
        {
            break;
        }
    }
	
//	if( i < ARRAY_SIZE(door_urc_table) )
	LOG_I("[Y]cmd[%d] found\r\n", i);
		
	return 0;
}
//************************************************************************************************************
//by yangwensen@20191114
static void cmd_open_door(const char *data, rt_size_t size)
{
	LOG_D("cmd_open_door[%d]\r\n", size);
}
//************************************************************************************************************
//by yangwensen@20191114
static void cmd_query_status(const char *data, rt_size_t size)
{
	LOG_D("cmd_query_status[%d]\r\n", size);
}
//************************************************************************************************************
//by yangwensen@20191114
static void cmd_update_soundcode(const char *data, rt_size_t size)
{
	LOG_D("cmd_update_soundcode[%d]\r\n", size);
}
//************************************************************************************************************
//by yangwensen@20191114
static void cmd_volume(const char *data, rt_size_t size)
{
	LOG_D("cmd_volume[%d]\r\n", size);
}
//************************************************************************************************************
