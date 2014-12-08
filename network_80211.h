#define FRAME_TYPE_MANAGEMENT 0
#define FRAME_TYPE_CONTROL 1
#define FRAME_TYPE_DATA 2
#define FRAME_SUBTYPE_PROBE_REQUEST 0x04
#define FRAME_SUBTYPE_PROBE_RESPONSE 0x05
#define FRAME_SUBTYPE_BEACON 0x08
#define FRAME_SUBTYPE_AUTH 0x0b
#define FRAME_SUBTYPE_DEAUTH 0x0c
#define FRAME_SUBTYPE_DATA 0x14
typedef struct framectrl_80211
{
    //buf[0]
    u8 Protocol:2;
    u8 Type:2;
    u8 Subtype:4;
    //buf[1]
    u8 ToDS:1;
    u8 FromDS:1;
    u8 MoreFlag:1;
    u8 Retry:1;
    u8 PwrMgmt:1;
    u8 MoreData:1;
    u8 Protectedframe:1;
    u8 Order:1;
} framectrl_80211,*lpframectrl_80211;

typedef struct probe_request_80211
{
	struct framectrl_80211 framectrl;
	uint16 duration;
	uint8 rdaddr[6];
	uint8 tsaddr[6];
	uint8 bssid[6];
	uint16 number;
} probe_request, *pprobe_request;

typedef struct tagged_parameter
{
	/* SSID parameter */
	uint8 tag_number;
	uint8 tag_length;
} tagged_parameter, *ptagged_parameter;
