#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "version.h"

void smartlink_success(void *args)
{
	printf("smartlink_success");
}
void smartlink_failed(void *args)
{
	printf("smartlink_failed");
}
SLIST_HEAD(router_info_head, router_info) router_list;

void ICACHE_FLASH_ATTR
test(void* args)
{
	os_printf("Hi, this is a callback function!");
}
//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
    uart_init(115200, 115200);
    smartlink_init(start_smartlink, NULL, smartlink_failed, 5);
}

