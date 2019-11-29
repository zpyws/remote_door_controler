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
#include <stdio.h>
#include <stdlib.h>

#include "led.h"
#include "relay.h"
#include <netdev.h>
#include <dfs_posix.h>
//************************************************************************************************************
static void urc_func(const char *data, rt_size_t size);
static void create_door_server_process(void);
int door_client_obj_init(void);
static int8_t door_register(int sock, char *str);
static int8_t check_response(char *str, uint8_t len, uint16_t sesson_id);
static int8_t recv_data_resolve(char *buff, uint32_t len);

//cmd
static void cmd_query_status(const char *data, rt_size_t size);
static void cmd_open_door(const char *data, rt_size_t size);
static void cmd_update_soundcode(const char *data, rt_size_t size);
static void cmd_volume(const char *data, rt_size_t size);
static void cmd_firmware_update(const char *data, rt_size_t size);
//************************************************************************************************************
extern int base64_decode(char * buf, int len);
extern int app_ec200t_start(void);
//************************************************************************************************************
#define BUFSZ   1024

#define SESSION_ID_LEN			4
#define DEVICE_SN_LEN			15//size(door_info.IMEI)
#define SOUND_MD5_LEN			32
#define MAX_SOUND_CLIPS			4
#define MAX_VERSION_STR_LEN		16

#define FIRMWARE_PACK_SIZE		512
#define FIRMWARE_FILE_NAME		"/FW.BIN"

door_info_t door_info;
int socket_tcp = -1;

//by yangwensen@20191112
static const struct at_urc door_urc_table[] = 
{
	{"OK:",		"\n", 	urc_func},
	{"ERR:",	"\n", 	urc_func},
	{"Q:",		"\n", 	cmd_query_status},
	{"O:",		"\n", 	cmd_open_door},
	{"S:",		"\n", 	cmd_update_soundcode},
	{"V:",		"\n", 	cmd_volume},
	{"U:",		"\n", 	cmd_firmware_update},
};

static const char FW_VERSION[] = "V1.0.0";

struct
{
	uint32_t size;
	uint32_t checksum;
}firmware_info;
//************************************************************************************************************
//#define DOOR_WRITE(a,b)		memdump((uint8_t *)(a),b)
#define DOOR_WRITE(a,b)		tcp_write(sock,(uint8_t *)(a),b)

#define SERVER_IP			"119.3.128.217"
#define SERVER_PORT			8090

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
	
	LOG_D("[MCU->SERVER][%d]: %.*s", len, len, buff);
	memdump(buff, len);
	ret = send(sock, buff, len, 0);
	if(ret!=len)LOG_E("[Y]TCP send %d bytes of %d\r\n", ret, len);
	else LOG_I("[Y]TCP send %d bytes\r\n", len);
	
	return ret;
}
//************************************************************************************************************
//by yangwensen@20191112
extern void door_init(void)
{
	app_ec200t_start();

	while(1)
	{
		LOG_D("[Y]EC200T linking...");
		if( netdev_is_link_up(netdev_get_by_name("ec20")) )break;
        rt_thread_mdelay(1000);
	}

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
//by yangwensen@20191121
static int8_t connect_server(char *server_ip, int server_port)
{
	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (connect(socket_tcp, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        LOG_E("[Y]Connect to dingdong home server fail!");
        return -1;
    }
	LOG_I("[Y]Connect to dingdong home server OK!");
	return 0;
}
//************************************************************************************************************
//by yangwensen@20191112
static uint8_t door_register_str(char *str)
{
	return rt_sprintf(str, "R:%04X:%s:%s:%s:%s\n", door_info.session_id, door_info.IMEI, door_info.ICCID, door_info.auth_code, "898604241118C0010270");
}
//************************************************************************************************************
//by yangwensen@20191121
static int8_t door_register(int sock, char *str)
{
	int len;
	int ret;

	//门锁注册
	len = door_register_str(str);

	ret = tcp_write(socket_tcp, (uint8_t *)str, len);
	if(ret != len)
		return -1;

	LOG_D("[Y]recv start=0x%08X\r\n", rt_tick_get());
	ret = recv(socket_tcp, str, BUFSZ - 1, 0);
	LOG_D("[Y]recv end=0x%08X\r\n", rt_tick_get());
	if(ret<=0)
	{
		LOG_E("[Y]recv error=%d\r\n", ret);
        return -2;
	}
	
	if( check_response(str, ret, door_info.session_id) != 0)
	{
		LOG_E("[Y]check server response data error\r\n");
        return -3;
	}

	LOG_I("[Y]door access controller registered!\r\n");
	return 0;
}
//************************************************************************************************************
//by yangwensen@20191114
static int8_t door_heart_beat(int sock, char *str)
{
	uint8_t len;
	int8_t error = 0;
	int ret;
	
	len = rt_sprintf(str, "H:%04X:%s:%d\n", 0xffff, door_info.IMEI, error);
	ret = tcp_write(sock, (uint8_t *)str, len);

	if(ret !=len)return -3;
#if 0	
	ret = recv(sock, str, BUFSZ - 1, 0);
	if(ret<=0)
	{
		LOG_E("[Y]heart beat response recv error=%d\r\n", ret);
        return -1;
	}

	if( check_response(str, ret, door_info.session_id) != 0)
	{
		LOG_E("[Y]check server response data error\r\n");
        return -2;
	}
	LOG_I("[Y]get heart beat pack response ok!\r\n");
#endif
	return 0;
}
//************************************************************************************************************
//by yangwensen@20191113
static int tcp_client(char *server_ip, int server_port)
{
	char *recv_data = RT_NULL;
	int ret;
	struct timeval timeout;
	
    recv_data = rt_malloc(BUFSZ);
    if (recv_data == RT_NULL)
    {
        LOG_E("[Y]tcp_client No memory[%d]\r\n", BUFSZ);
        return -1;
    }
//=====================================================================================	
	socket_tcp = socket(AF_AT, SOCK_STREAM, 0);
	if(socket_tcp < 0)
	{
		LOG_E("[Y]Create socket error");
		goto __TCP_CLIENT_EXIT;
	}
	LOG_I("[Y]Socket allocated ok\n");
//=====================================================================================	
	while(1)
	{
		if( connect_server(server_ip, server_port) == 0)
			break;
        rt_thread_mdelay(500);
	}
//=====================================================================================	
	//门锁注册
	while(1)
	{
		if( door_register(socket_tcp, recv_data) == 0)
			break;
        rt_thread_mdelay(500);
	}

	door_heart_beat(socket_tcp, recv_data);
//=====================================================================================	
	timeout.tv_sec = 30;
	timeout.tv_usec =  0;
	setsockopt(socket_tcp, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
	while(1)
	{
		ret = recv(socket_tcp, recv_data, BUFSZ - 1, 0);
		
		if(ret==-1 && errno==EAGAIN)
		{
			LOG_W("[Y]recv timeout,send heart beat pack\r\n");
			door_heart_beat(socket_tcp, recv_data);
			continue;
		}
		else if(ret==0)		//current socket closed by remote
		{
			socket_tcp = -1;
			LOG_E("[Y]socket closed by remote host\r\n");
			break;
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
	
	if(socket_tcp >=0)
	{
		closesocket(socket_tcp);
		socket_tcp = -1;
	}
	return -1;
}
//************************************************************************************************************
//by yangwensen@20191113
static void task_door_server(void* parameter)
{
	
	while(1)
	{
		tcp_client(SERVER_IP, SERVER_PORT);
        rt_thread_mdelay(1000);	
		LOG_E("[Y]****************socket closed*********************\r\n");
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
	LOG_D("[SERVER->MCU][%d]: %.*s", buf_sz, buf_sz, buffer);
	
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
	
	if( i >= ARRAY_SIZE(door_urc_table) )
		return -1;
	
//	LOG_D("[Y]cmd[%d] found\r\n", i);
	
	if(door_urc_table[i].func == RT_NULL)
		return -2;
	
	door_urc_table[i].func( &buffer[prefix_len], buf_sz-(prefix_len+suffix_len) );

	return 0;
}
//************************************************************************************************************
//by yangwensen@20191118
const char *resp_get_field(char *resp_buf, rt_size_t len, rt_size_t resp_line)
{
    rt_size_t line_num = 0;
	rt_size_t i;

    RT_ASSERT(resp_buf);

	if(resp_line==0)return resp_buf;

	for(i=0; i<len; i++)
	{
		if(*resp_buf == ':')
		{
			if(++line_num>=resp_line)
			{
				return (resp_buf+1);
			}
		}
		resp_buf++;
	}

    return RT_NULL;
}
//************************************************************************************************************
//by yangwensen@20191114
int door_resp_parse_line_args(const char *resp_line_buf, const char *resp_expr, ...)
{
    va_list args;
    int resp_args_num = 0;

    RT_ASSERT(resp_expr);

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

//************************************************************************************************************
//by yangwensen@20191114
static void cmd_open_door(const char *data, rt_size_t size)
{
	char str[4+SESSION_ID_LEN+DEVICE_SN_LEN+2];
	uint8_t len;
	
	LOG_D("cmd_open_door[%d]\r\n", size);
	
	len = rt_sprintf(str, "OK:%04X:%s\n", 0, door_info.IMEI);
	rt_memcpy(&str[3], data, SESSION_ID_LEN);
	tcp_write(socket_tcp, (uint8_t *)str, len);

	door_open();
}
//************************************************************************************************************
//by yangwensen@20191114
static void cmd_query_status(const char *data, rt_size_t size)
{
	#define CMD_QUERY_STATUS_RESPONSE_LEN_MAX	(3+SESSION_ID_LEN+DEVICE_SN_LEN+sizeof(FW_VERSION)+5+(1+SOUND_MD5_LEN+3+3+3)*MAX_SOUND_CLIPS)

	char *buf;
	rt_size_t len;
	uint8_t signal_strenth;
	uint8_t i;

	LOG_D("cmd_query_status[%d]\r\n", size);

    buf = rt_malloc(CMD_QUERY_STATUS_RESPONSE_LEN_MAX);
    if (buf == RT_NULL)
    {
        LOG_E("cmd_query_status No memory[%d]\r\n", CMD_QUERY_STATUS_RESPONSE_LEN_MAX);
        return;
    }
//----------------------------------------------------------------------------------------------
	len = rt_sprintf(buf, "OK:%04X:%s:%d:%s", door_info.session_id, door_info.IMEI, signal_strenth, FW_VERSION);
	rt_memcpy(&buf[3], data, SESSION_ID_LEN);
	for(i=0; i<MAX_SOUND_CLIPS; i++)
	{
		len += rt_sprintf(buf+len, ":%d:%s:%d", i, "898604241118C0010270", 55+i);	//index,md5,volume
	}
	buf[len++] = '\n';

	tcp_write(socket_tcp, (uint8_t *)buf, len);
//----------------------------------------------------------------------------------------------
	rt_free(buf);
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
	#define CMD_VOLUME_LEN_MAX	(3+SESSION_ID_LEN+DEVICE_SN_LEN+4+10)

	char *buf;
	rt_size_t len;
	int index,volume;

	LOG_D("cmd_volume[%d]\r\n", size);

    buf = rt_malloc(CMD_VOLUME_LEN_MAX);
    if (buf == RT_NULL)
    {
        LOG_E("cmd_volume No memory[%d]\r\n", CMD_VOLUME_LEN_MAX);
        return;
    }
//----------------------------------------------------------------------------------------------
	index = atoi( resp_get_field((char *)data, size, 1) );
	volume = atoi( resp_get_field((char *)data, size, 2) );
	LOG_I("cmd_volume[%d][%d]\r\n", index, volume);
	len = rt_sprintf(buf, "OK:%04X:%s:%d:%d\n", door_info.session_id, door_info.IMEI, index, volume);
	rt_memcpy(&buf[3], data, SESSION_ID_LEN);

	tcp_write(socket_tcp, (uint8_t *)buf, len);
//----------------------------------------------------------------------------------------------
	rt_free(buf);
}
//************************************************************************************************************
//by yangwensen@20191120
static uint32_t calc_checksum(char *buff, uint32_t len)
{
	uint32_t sum = 0;

	while(len--)
	{
		sum += *buff++;
	}
	return sum;
}
//************************************************************************************************************
//by yangwensen@20191120
static void cmd_firmware_update(const char *data, rt_size_t size)
{
	#define FIRMWARE_PACK_SHELL_SIZE		(3+SESSION_ID_LEN+DEVICE_SN_LEN+8+8+7)	//协议开销
	#define FIRMWARE_PACK_BASE64_SIZE		((FIRMWARE_PACK_SIZE/3+1)<<2)
	#define FIRMWARE_PACK_TOTAL_SIZE		(FIRMWARE_PACK_SHELL_SIZE+FIRMWARE_PACK_BASE64_SIZE)

	char *buf;
	uint32_t len;
	char session[SESSION_ID_LEN+1];
	uint32_t offset = 0;
	uint32_t packs;
	uint32_t i;
	int ret;
	uint32_t sum = 0;
	char *p,*p1;
	char version[MAX_VERSION_STR_LEN];
	int fd = -1;
	int result;
	uint32_t pack_checksum;

	LOG_D("cmd_firmware_update[%d]\r\n", size);

    buf = rt_malloc(FIRMWARE_PACK_TOTAL_SIZE);
    if (buf == RT_NULL)
    {
        LOG_E("cmd_firmware_update No memory[%d]\r\n", FIRMWARE_PACK_TOTAL_SIZE);
        return;
    }

	rt_memcpy(session, data, SESSION_ID_LEN);
	session[SESSION_ID_LEN] = 0;

	p = (char *)resp_get_field((char *)data, size, 1);		//get version field
	if(p==RT_NULL)goto cmd_firmware_update_2;

	p1 = (char *)resp_get_field((char *)data, size, 2);		//get checksum field
	if(p1==RT_NULL)goto cmd_firmware_update_2;

	//get version string
	len = (p1-p > MAX_VERSION_STR_LEN) ? MAX_VERSION_STR_LEN-1 : (p1-p-1);
	rt_memcpy(version, p, len);
	version[len] = 0;

	firmware_info.checksum = atoi( p1 );
	firmware_info.size = atoi( resp_get_field((char *)data, size, 3) );
	LOG_I("DFU[%6s][sum=0x%08X][size=%d]\r\n", p, firmware_info.checksum, firmware_info.size );

	packs = firmware_info.size / FIRMWARE_PACK_SIZE;
//----------------------------------------------------------------------------------------------
	fd = open(FIRMWARE_FILE_NAME, O_CREAT|O_WRONLY|O_TRUNC);
	if (fd < 0)
	{
		LOG_E("open file for write failed\n");
		goto cmd_firmware_update_2;
	}
//----------------------------------------------------------------------------------------------
	for(i=0; i<packs; i++)
	{
		LOG_D("[Y]Request Firmware Pack %d of %d, size=%d", i+1, packs, FIRMWARE_PACK_SIZE);
		len = rt_sprintf(buf, "U:%s:%s:%s:%d:%d\n", session, door_info.IMEI, version, offset, FIRMWARE_PACK_SIZE);
cmd_firmware_update_1:
		tcp_write(socket_tcp, (uint8_t *)buf, len);
//----------------------------------------------------------------------------------------------
		ret = recv(socket_tcp, buf, FIRMWARE_PACK_TOTAL_SIZE-1, 0);
		if(ret==-1 && errno==EAGAIN)
		{
			LOG_E("[Y]firmware recv timeout\r\n");
			ret = -1;
			break;
		}
		else if(ret==0)		//current socket closed by remote
		{
			socket_tcp = -1;
			ret = -2;
			LOG_E("[Y]socket closed by remote host\r\n");
			goto cmd_firmware_update_0;
		}
//----------------------------------------------------------------------------------------------
		LOG_D("[SERVER->MCU][%d]: %.*s", ret, ret, buf);

		//get session id
		rt_memcpy(session, resp_get_field((char *)buf, ret, 1), SESSION_ID_LEN);
		session[SESSION_ID_LEN] = 0;

		//get sub pack checksum
		pack_checksum = atoi( resp_get_field((char *)buf, ret, 2) );

		//base64 decode
		p = (char *)resp_get_field((char *)buf, ret, 3);	//get base64 data field
		if(p==RT_NULL)										//illeagal data structure
		{
			LOG_E("[Y]dfu get base64 field failed\r\n");
			ret = -3;
			goto cmd_firmware_update_2;			
		}

		ret = base64_decode(p, ret-(p-buf)-1);
		if(ret<0)
		{
			LOG_E("[Y]dfu base64 decode failed[%d]", ret);
			ret = -4;
			goto cmd_firmware_update_2;			
		}

		LOG_I("[Y]Firmware Pack %d of %d, size=%d", i, packs, ret);
		offset += ret;
		len = calc_checksum(p, ret);		//sub pack checksum
		if(len!=pack_checksum)
		{
			LOG_E("[DFU][sub-checksum]SRV=%d,MCU=%d", pack_checksum, len);
			ret = -5;
			goto cmd_firmware_update_2;
		}
		sum += len;

//		memdump((uint8_t *)p, ret);
		result = write(fd, p, ret);
		if(result==-1)
		{
			LOG_E("[DFU]file write failed %d", errno);
			goto cmd_firmware_update_2;
		}
		LOG_I("[DFU]Pack %d//%d written %d bytes to file", i, packs, result);
	}
//----------------------------------------------------------------------------------------------
	if(i<packs)							//recieve timeout
	{
cmd_firmware_update_2:
		LOG_E("[Y]dfu unsuccessfully[%d]", ret);
		len = rt_sprintf(buf, "ERR:%s\n", session);
		tcp_write(socket_tcp, (uint8_t *)buf, len);
		rt_free(buf);
		if(fd>=0)close(fd);
		return;
	}
//----------------------------------------------------------------------------------------------
	if(offset < firmware_info.size)		//do we have the last packet?
	{
		len = rt_sprintf(buf, "U:%s:%s:%s:%08x:%08x\n", session, door_info.IMEI, version, offset, firmware_info.size % FIRMWARE_PACK_SIZE);
		LOG_D("[Y]Request Last Firmware Pack, size=%d\r\n", firmware_info.size % FIRMWARE_PACK_SIZE);
		goto cmd_firmware_update_1;
	}
//----------------------------------------------------------------------------------------------
	if(sum!=firmware_info.checksum)
	{
		LOG_E("[DFU][checksum]SRV=%d,MCU=%d", firmware_info.checksum, sum);
		goto cmd_firmware_update_2;
	}
//----------------------------------------------------------------------------------------------
	//firmware download successfully
	LOG_I("[Y]Firmware Download Successful,total size %d bytes\r\n", firmware_info.size);
	len = rt_sprintf(buf, "OK:%s:%s:%s\n", session, door_info.IMEI, version);
	tcp_write(socket_tcp, (uint8_t *)buf, len);

cmd_firmware_update_0:
	rt_free(buf);
	close(fd);
}
//************************************************************************************************************
