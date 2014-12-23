#include "c_types.h"
#include "osapi.h"
#include "network_80211.h"
#include "user_interface.h"

#define SMARTLK_TOKEN "<!-SL-!>"
#define SMARTLK_TOKEN_LEN (strlen(SMARTLK_TOKEN))

void smartlink_wifi_promiscuous_rx(uint8_t*, uint16);
void smartlink_received(char*, char*);
void delay_ms(uint16);
void delay_s(uint16);

void (*smartlink_received_cb)(void*) = NULL;
void *smartlink_received_cb_args = NULL;
void (*smartlink_failed_cb)() = NULL;

uint8_t max_wait = 5;

void delay_s(uint16 ss)
{
	uint16 i;
	for(i = 0; i < ss; i++)
	{
		delay_ms(1000);
	}	
}
void delay_ms(uint16 sms)
{
	uint16 i;
	for(i = 0; i < sms; i++)
	{
		os_delay_us(1000);
	}
}
void ICACHE_FLASH_ATTR
smartlink_init(void* cb, void* args, void* fail, uint8_t max)
{	
	os_printf("\n\n");
	os_printf("[*] reset wifi config \n");
	{
		wifi_station_disconnect();
		wifi_station_set_config(NULL);
	}

	os_printf("[*] set opmode=0x01\n");
    wifi_set_opmode(0x01);
    
    os_printf("[*] set smartlink_received callback function\n");
    smartlink_received_cb = cb;
    smartlink_received_cb_args = args;
    smartlink_failed_cb = fail;
    max_wait = max;

    os_printf("[*] enable wifi promiscuous\n");
    wifi_promiscuous_enable(1);
    wifi_set_promiscuous_rx_cb(smartlink_wifi_promiscuous_rx);
}

void ICACHE_FLASH_ATTR
smartlink_wifi_promiscuous_rx(uint8_t *buf, uint16 len)
{

    uint16 i;
    uint8_t type;
    
   // os_printf("smartlink_wifi_promiscuous_rx -----\n");
    lpframectrl_80211 framectrl;
    struct router_info *info = NULL;
    if (len < 64) {
        return;
    }
    struct sniffer_buf *sniffer = (struct sniffer_buf*)buf;
    buf +=sizeof(struct RxControl);
    struct probe_request_80211 *probe_buf = (struct probe_request_80211*)buf;
	if (FRAME_TYPE_MANAGEMENT == probe_buf->framectrl.Type) {
    	/* Management frame */
    	if (FRAME_SUBTYPE_PROBE_REQUEST == probe_buf->framectrl.Subtype) {
          
    		/* Probe Request */
    		ptagged_parameter tag = (ptagged_parameter)(buf + sizeof(probe_request));
    		if (tag->tag_length != 0)
    		{
	    		uint8_t ssid_buff[32];
	    		os_memset(ssid_buff, 0, 32);
	    		os_memcpy(ssid_buff, (uint8_t *)tag + 2, tag->tag_length);
	    		char *pos = (char *)os_strstr(ssid_buff, SMARTLK_TOKEN);
	    		if (pos != NULL)
	    		{
	    			uint8_t ssid[40];
	    			uint8_t password[40];
	    			os_memset(ssid, 0, 32);
	    			os_memset(password, 0, 32);
	    			/* ssid */
	    			os_memcpy(ssid, ssid_buff, pos - (char *)ssid_buff);
	    			/* password */
	    			os_memcpy(password, pos + SMARTLK_TOKEN_LEN, tag->tag_length - SMARTLK_TOKEN_LEN - strlen(ssid));
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
	//wifi_promiscuous_enable(0);
	os_printf("[*] change mode: STATION \n");
	wifi_set_opmode(0x01);
	{
		struct station_config config;
		os_strcpy(config.ssid, ssid);
		os_strcpy(config.password, password);
		wifi_station_set_config(&config);
	}
    system_restart();
#if 0
	os_printf("[*] connecting to %s ... \n", ssid);
	wifi_station_connect();

	os_printf("[*] check connection to %s ... \n", ssid);
	{
		uint8_t status;
		uint8_t retry;
		for(retry = 0; retry < max_wait; ++retry)
		{
			status = wifi_station_get_connect_status();
			if (STATION_IDLE == status || STATION_GOT_IP == status)
			{	
				os_printf("[*] connected to %s", ssid);
				if (smartlink_received_cb)
				{
					(*smartlink_received_cb)(smartlink_received_cb_args);
				}
			} else {
				os_printf("[*] probing AP:%s\n", ssid);
				//vTaskDelay(1000 / portTICK_RATE_MS);			
			}
		}	
		
	}

	if (smartlink_failed_cb)
	{
		(*smartlink_failed_cb)();
	}	
#endif
}
