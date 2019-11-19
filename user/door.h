#ifndef DOOR_H
#define DOOR_H
//************************************************************************************************************
//by yangwensen@20191112
typedef struct
{
	uint8_t IMEI[15+1];
	uint8_t ICCID[20+1];
	
	uint8_t auth_code[9+1];
	uint16_t session_id;

	uint8_t volume;
}door_info_t;
//************************************************************************************************************
extern door_info_t door_info;
//************************************************************************************************************
extern void door_init(void);
//************************************************************************************************************
#endif
