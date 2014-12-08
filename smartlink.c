#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "network_80211.h"
#define SMARTLK_TOKEN "<!-SL-!>"
#define SMARTLK_TOKEN_LEN (os_strlen(SMARTLK_TOKEN))

void smartlink_wifi_promiscuous_rx(uint8*, uint16);
void smartlink_received(char*, char*);

void (*smartlink_received_cb)(void*) = NULL;
void *smartlink_received_cb_args = NULL;

void smartlink_init(void* cb, void* args)
{	
	os_printf("\n\n");
	os_printf("[*] reset wifi config \n");
	{
		wifi_station_set_config(NULL);
	}

	os_printf("[*] set opmode=0x01\n");
    wifi_set_opmode(0x01);
    
    os_printf("[*] set smartlink_received callback function\n");
    smartlink_received_cb = cb;
    smartlink_received_cb_args = args;

    os_printf("[*] enable wifi promiscuous\n");
    wifi_promiscuous_enable(1);
    wifi_set_promiscuous_rx_cb(smartlink_wifi_promiscuous_rx);
}

void ICACHE_FLASH_ATTR
smartlink_wifi_promiscuous_rx(uint8 *buf, uint16 len)
{
    uint16 i;
    uint8 type;

    lpframectrl_80211 framectrl;
    struct router_info *info = NULL;
    framectrl = (lpframectrl_80211)buf;
	if (FRAME_TYPE_MANAGEMENT == framectrl->Type) {
    	/* Management frame */
    	if (FRAME_SUBTYPE_PROBE_REQUEST == framectrl->Subtype) {
    		/* Probe Request */
    		ptagged_parameter tag = (ptagged_parameter)(buf + sizeof(probe_request));
    		if (tag->tag_length != 0)
    		{
	    		uint8 ssid_buff[32];
	    		os_memset(ssid_buff, 0, 32);
	    		os_memcpy(ssid_buff, (uint8 *)tag + 2, tag->tag_length);
	    		char *pos = strstr(ssid_buff, SMARTLK_TOKEN);
	    		if (pos != NULL)
	    		{
	    			uint8 ssid[40];
	    			uint8 password[40];
	    			os_memset(ssid, 0, 32);
	    			os_memset(password, 0, 32);
	    			/* ssid */
	    			os_memcpy(ssid, ssid_buff, (uint8*)pos - ssid_buff);
	    			/* password */
	    			os_memcpy(password, pos + SMARTLK_TOKEN_LEN, tag->tag_length - SMARTLK_TOKEN_LEN - os_strlen(ssid));
	    			smartlink_received(ssid, password);
	    		}
    		}
    	}
	}
}
void ICACHE_FLASH_ATTR
smartlink_received(char* ssid, char* password)
{
	os_printf("[*] received SSID:%s PASSWORD:%s\n", ssid, password);
	wifi_promiscuous_enable(0);
	os_printf("[*] change mode: AP \n");
	wifi_set_opmode(0x02);

	if (smartlink_received_cb)
	{
		(*smartlink_received_cb)(smartlink_received_cb_args);
	}
}
