//created by yangwensen@20191125
#include <rtthread.h>
//***************************************************************************************
extern void memdump(uint8_t *buff, uint16_t len);

#define BASE64_DBG		rt_kprintf
//***************************************************************************************
#if 0
static const char base64[64] = 
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};
#endif

static const int8_t TAB_BASE64_DECODE[128] = 
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, 
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
};
//***************************************************************************************
//by yangwensen@20191125
static int8_t base64_decrypt(const char * cbuf, char * pbuf)
{
    uint32_t temp = 0;
	uint8_t i;
	int8_t c;
	uint8_t valid = 0;		//解码前的有效数据个数

    for(i=0; i<4; i++)
	{
		c = cbuf[i];
		BASE64_DBG("[%02X.", c);
		if(c & 0x80)return -1;

		c = TAB_BASE64_DECODE[c];
		if(c==-1)return -2;
		BASE64_DBG("%02X]", c);

		temp <<= 6;
		if(c!=-2)			//get '='
		{
			temp |= c;
			valid++;
		}
    }

	BASE64_DBG("--%d-->", valid);
	
	if(valid<2)return -3;

	valid--;		//解码有效字节数
    for(i=0; i<valid; i++)
	{
        pbuf[i] = (temp >> (16-(i<<3))) & 0xfful;
		BASE64_DBG("[%02X]", pbuf[i]);
    }
	BASE64_DBG("\n");
	
	return valid;
}
//***************************************************************************************
//by yangwensen@20191125
extern int8_t base64_decode(char * buf, int len)
{
	char *pbuf;
	int plen;
	int i;
	uint32_t n = 0;
	int8_t ret;

	if(len & 3)return -1;

    plen = len / 4;

    pbuf = rt_malloc(plen*3);
    if (pbuf == RT_NULL)
    {
        rt_kprintf("No memory\r\n");
        return -2;
    }

    rt_memset(pbuf, 0, plen*3);

    for(i=0; i < plen; i++)
	{
		ret = base64_decrypt(&buf[i*4], &pbuf[i*3]);
        if( ret < 0)return -3;
		n += ret;
    }
	rt_kprintf("before decode\r\n");
    memdump((uint8_t *)buf, len);
	rt_kprintf("after decode\r\n");
    memdump((uint8_t *)pbuf, n);

    rt_free(pbuf);
	return plen*3;
}
/**
*/
//***************************************************************************************
void base64_test(void)
{
	const char BASE64_STR[] = "MTIzNDU2Nzg5MA==";
	const char BASE64_STR2[] = "MTIzNDU2Nzg5MDk=";
	const char BASE64_STR3[] = "MTIzNDU2Nzg5MDk4";

	base64_decode((char *)BASE64_STR, sizeof(BASE64_STR)-1);
	base64_decode((char *)BASE64_STR2, sizeof(BASE64_STR2)-1);
	base64_decode((char *)BASE64_STR3, sizeof(BASE64_STR3)-1);
}
//***************************************************************************************
