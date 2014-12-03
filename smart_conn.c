/*
    originated from http://blog.sina.com.cn/s/blog_69f669470102vf1m.html
*/
#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"

#include "user_interface.h"

#include "version.h"

typedef enum _encrytion_mode {
    ENCRY_NONE           = 1,
    ENCRY_WEP,
    ENCRY_TKIP,
    ENCRY_CCMP
} ENCYTPTION_MODE;

struct router_info {
    SLIST_ENTRY(router_info)     next;

    u8 bssid[6];
    u8 channel;
    u8 authmode;

    u16 rx_seq;
    u8 encrytion_mode;
    u8 iv[8];
    u8 iv_check;
};

//smartlink var def begin
enum enumSL_Checking_stage
{
    SL_CH_STAGE_INIT = 0x1,
    SL_CH_STAGE_SENDER_LOCKING = 0x2,
    SL_CH_STAGE_SENDER_LOCKED = 0x4
};

#define RECEVEBUF_MAXLEN (90)
#define SCAN_TIMEINTERVAL (1500)

u8 SL_sendMacLock=0;
u8 SL_sendMac[6];
int SL_sendChannel=-1;
uint32 SL_sendMacLockTime=0;
u8 SL_protocol_OnByte[10];
u8 SL_protocol_OnByteCurIdx=0;
u8 SL_protocol_ReceveBuf[RECEVEBUF_MAXLEN];//todo use os_malloc();
u8 SL_protocol_ReceveBuf_Len=0;
u8 SL_Checking_stage = SL_CH_STAGE_INIT;

//smartlink var def end

//smartlink fun def begin
unsigned char calcrc_1byte(unsigned char abyte)    
{    
    unsigned char i,crc_1byte;     
    crc_1byte=0;                //设定crc_1byte初值为0  
    for(i = 0; i < 8; i++)    
     {    
       if(((crc_1byte^abyte)&0x01))    
          {    
            crc_1byte^=0x18;     
            crc_1byte>>=1;    
            crc_1byte|=0x80;    
           }          
        else     
           crc_1byte>>=1;   
        abyte>>=1;          
      }   
      return crc_1byte;   
}   

unsigned char calcrc_bytes(unsigned char *p,unsigned char len)  
{  
    unsigned char crc=0;  
    while(len--) //len为总共要校验的字节数  
    {  
        crc=calcrc_1byte(crc^*p++);  
    }  
    return crc;  //若最终返回的crc为0，则数据传输正确  
}
   
int SmartLinkDecode(u8* pOneByte)
{
    int i = 0;
    u8 pos=0,val=0;
    u8 n0,n1;

    if( !(0==pOneByte[0]&&0==pOneByte[1]) ) return -9;

    n0 = (pOneByte[2]-1);
    n1 = (pOneByte[3]-1);
    if( (n1+n0) != 15 )
    {
        os_printf("n0=%d,n1=%d\n",n0,n1);
        return -1;
    }
    pos = (n0&0xf);
    n0 = (pOneByte[4]-1);
    n1 = (pOneByte[5]-1);

    if( (n1+n0) != 15 ) return -2;

    pos |= (n0&0xf)<<4;
    n0 = (pOneByte[6]-1);
    n1 = (pOneByte[7]-1);

    if( (n1+n0) != 15 ) return -3;

    val = (n0&0xf);
    n0 = (pOneByte[8]-1);
    n1 = (pOneByte[9]-1);

    if( (n1+n0) != 15 ) return -4;

    val |= (n0&0xf)<<4;
    if( (int)pos >= (int)RECEVEBUF_MAXLEN )
        return -5;

    if( SL_protocol_ReceveBuf[pos] == val && SL_protocol_ReceveBuf_Len>2 )
    {//begin crc8
        if(  calcrc_bytes(SL_protocol_ReceveBuf,SL_protocol_ReceveBuf_Len) == SL_protocol_ReceveBuf[SL_protocol_ReceveBuf_Len] )
        {
            SL_protocol_ReceveBuf[SL_protocol_ReceveBuf_Len+1]='\0';
            os_printf("\ngot ssid-pwd show it\n");
            os_printf("%s\n",SL_protocol_ReceveBuf);
            return 0;
        }
    }
    else
    {
        SL_protocol_ReceveBuf[pos] = val;
        os_printf("[%d]=%d\n",pos,val);
        if( pos > SL_protocol_ReceveBuf_Len )
        {
            SL_protocol_ReceveBuf_Len = pos;
            os_printf("new LEN=%d\n",SL_protocol_ReceveBuf_Len);
        }
        return SL_CH_STAGE_SENDER_LOCKED;
    }
    return 99;
}

//buf point to MAC
void SL_Checker(u8* buf,int len,int channel)
{
    int funret;
    if( len > 16 )
        return;
    if( (system_get_time() - SL_sendMacLockTime) > 70000)
    {
        SL_Checking_stage =SL_CH_STAGE_INIT; 
        os_printf("into SL_CH_STAGE_INIT\n");
    }
    if(SL_CH_STAGE_INIT == SL_Checking_stage && len == 0)
    {
    if(SL_sendMacLock == 0)
    {
        os_memcpy(SL_sendMac,buf,6);}
        SL_sendChannel=channel;
        SL_sendMacLockTime=system_get_time();
        SL_Checking_stage = SL_CH_STAGE_SENDER_LOCKING;
        SL_protocol_OnByteCurIdx = 0;
        SL_protocol_OnByte[SL_protocol_OnByteCurIdx++]=len;
        os_printf("into SL_CH_STAGE_SENDER_LOCKING\n");
    }
    else if( SL_CH_STAGE_SENDER_LOCKING == SL_Checking_stage )
    {
        if( os_memcmp(SL_sendMac,buf,6) == 0 )
        {
            if( len == 0)
            {
                SL_protocol_OnByte[SL_protocol_OnByteCurIdx++]=len;
                SL_Checking_stage = SL_CH_STAGE_SENDER_LOCKED;
                os_printf("into SL_CH_STAGE_SENDER_LOCKED\n");
            }
            else
            {
                SL_Checking_stage =SL_CH_STAGE_INIT; 
                SL_protocol_OnByteCurIdx=0;
            }
                SL_sendMacLockTime=system_get_time();
        }
    }
    else if( SL_CH_STAGE_SENDER_LOCKED == SL_Checking_stage )
    {
        if( os_memcmp(SL_sendMac,buf,6) == 0 )
        {
            if( len == 0 )
            {
                if(SL_sendMacLock == 0)
                {
                    os_memcpy(SL_sendMac,buf,6);
                }
                SL_sendChannel=channel;
                SL_Checking_stage = SL_CH_STAGE_SENDER_LOCKING;
                SL_protocol_OnByteCurIdx = 0;
                SL_protocol_OnByte[SL_protocol_OnByteCurIdx++]=len;
                os_printf("resync  into SL_CH_STAGE_SENDER_LOCKING\n");
            }
            else
            {
                SL_protocol_OnByte[SL_protocol_OnByteCurIdx++]=len; 
            }
            SL_sendMacLockTime=system_get_time();
        }
    }
    if(SL_protocol_OnByteCurIdx>=10)
    {
        funret = SmartLinkDecode(SL_protocol_OnByte);
        os_printf("Decode ret=%d\n",funret);
        if( SL_CH_STAGE_SENDER_LOCKED == funret )
        {
            SL_Checking_stage = SL_CH_STAGE_SENDER_LOCKED ;
            SL_sendMacLock = 1;
        }
        SL_Checking_stage =SL_CH_STAGE_INIT; 
        SL_protocol_OnByteCurIdx=0;
    }
}

//smartlink fun def end



SLIST_HEAD(router_info_head, router_info) router_list;

os_timer_t channel_timer;
uint8 current_channel;
uint16 channel_bits;

typedef struct framectrl_80211
{
    //buf[0]
    u8 Subtype:4;
    u8 Type:2;
    u8 Protocol:2;
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

void wifi_scan_done(void *arg, STATUS status);

void ICACHE_FLASH_ATTR
wifi_promiscuous_rx(uint8 *buf, uint16 len)
{
    uint16 i;
    lpframectrl_80211 framectrl;
    struct router_info *info = NULL;
    framectrl = (lpframectrl_80211)buf;
    os_printf("RA:" MACSTR ",", MAC2STR(buf + 4), len);
    os_printf("SA:" MACSTR ",", MAC2STR(buf + 4+6), len);
    os_printf("DA:" MACSTR ",len %d\n", MAC2STR(buf + 4+6+6), len);
    SLIST_FOREACH(info, &router_list, next) {
        if ((buf[1] & 0x01) == 0x01) { // just toDS
            if (os_memcmp(info->bssid, buf + 4, 6) == 0) {
                if (current_channel - 1 != info->channel) { // check channel
                    return;
                } else {
                    break;
                }
            }
        }
    }

    if (info == NULL) {
    return;
    }

    if ((buf[0] == 0x08 || buf[0] == 0x88) && (buf[1] & 0x01) == 0x01) {
        //only data frame and ToDS==1 只分析数据帧，会容易处理一些
        if (info->rx_seq != (*(uint16 *)(buf + 22)) >> 4) {
            info->rx_seq = (*(uint16 *)(buf + 22)) >> 4;

            if (info->encrytion_mode != 0) {
            if ((buf[0] & 0x80) == 0) {
                len -= 24;
            } else {
                len -= 26;
            }
            if (info->encrytion_mode == ENCRY_NONE) {
                len -= 40;
            } else if (info->encrytion_mode == ENCRY_WEP){
                len = len - 40 - 4 - 4;
            } else if (info->encrytion_mode == ENCRY_TKIP) {
                len = len - 40 - 12 - 8;
            } else if (info->encrytion_mode == ENCRY_CCMP) {
                len = len - 40 - 8 - 8;
            }

            if (len == 3 || len == 23 || (len >= 593 && len <= 848) || len == 1099 || len == 1199 || (len > 28 && len <= 60)) {
            if (len >= 593 && len <= 848) {
            len -= 593;
                os_printf(MACSTR ",len --- d x\n", MAC2STR(buf + 4), len + 593, len);
            } else {
                os_printf(MACSTR ",len %d\n", MAC2STR(buf + 4), len);
            }
            } else {
              os_printf(MACSTR ",len --- %d\n", MAC2STR(buf + 4), len);
            }
            os_printf("%d\n", len);
            //Smartlink checker 
            SL_Checker(buf+4,len,info->channel);//对长度进行协议测试
            } else {
                if (info->authmode == AUTH_OPEN) {
                info->encrytion_mode = ENCRY_NONE;
                os_printf(MACSTR ", channel %d, mode %d\n", MAC2STR(info->bssid), info->channel, info->encrytion_mode);
                } else if (info->authmode == AUTH_WEP) {
                info->encrytion_mode = ENCRY_WEP;
                os_printf(MACSTR ", channel %d, mode %d\n", MAC2STR(info->bssid), info->channel, info->encrytion_mode);
                } else {
                    if (info->iv_check == 0) {
                        if (buf[0] == 0x08) {
                            os_memcpy(info->iv, buf + 24, 8);
                        } else if (buf[0] == 0x88) {
                            os_memcpy(info->iv, buf + 26, 8);
                        }
                            info->iv_check = 1;
                    } else {
                        uint8 *local_iv;
                        if (buf[0] == 0x08) {
                            local_iv = buf + 24;
                        } else if (buf[0] == 0x88) {
                            local_iv = buf + 26;
                        }
                            if (info->iv[2] == local_iv[2] && local_iv[2] == 0) {
                            info->encrytion_mode = ENCRY_CCMP;
                            os_printf(MACSTR ", channel %d, mode %d\n", MAC2STR(info->bssid), info->channel, info->encrytion_mode);
                        } else {
                            info->encrytion_mode = ENCRY_TKIP;
                            os_printf(MACSTR ", channel %d, mode %d\n", MAC2STR(info->bssid), info->channel, info->encrytion_mode);
                        }
                    }
                }
            }
        }
    }
}

void ICACHE_FLASH_ATTR
channel_timer_cb(void *arg)
{
    uint8 i;
    if( SL_sendChannel>0 && SL_sendMacLock == 1)
    {
        wifi_set_channel(SL_sendChannel);
        os_printf("locked Smartlink channel=%d\n",SL_sendChannel);
    return;
    }
    for (i = current_channel; i < 14; i++) {
        if ((channel_bits & (1 << i)) != 0) {
            current_channel = i + 1;
            wifi_set_channel(i);
            os_printf("current channel2 %d--------------------------------------------%d\n", i, system_get_time());
            os_timer_arm(&channel_timer, SCAN_TIMEINTERVAL, 0);
            break;
        }
    }

    if (i == 14) {
        current_channel = 1;
        for (i = current_channel; i < 14; i++) {
            if ((channel_bits & (1 << i)) != 0) {
                current_channel = i + 1;
                wifi_set_channel(i);
                os_printf("current channel3 %d--------------------------------------------%d\n", i, system_get_time());
                os_timer_arm(&channel_timer, SCAN_TIMEINTERVAL, 0);
                break;
            }
        }
    }
}

void ICACHE_FLASH_ATTR
wifi_scan_done(void *arg, STATUS status)
{
    uint8 ssid[33];

    channel_bits = 0;
    current_channel = 1;

    struct router_info *info = NULL;

    while((info = SLIST_FIRST(&router_list)) != NULL){
        SLIST_REMOVE_HEAD(&router_list, next);

        os_free(info);
    }

    if (status == OK) {
        uint8 i;
        struct bss_info *bss = (struct bss_info *)arg;

        while (bss != NULL) {
        os_memset(ssid, 0, 33);

        if (os_strlen(bss->ssid) <= 32) {
        os_memcpy(ssid, bss->ssid, os_strlen(bss->ssid));
    } else {
        os_memcpy(ssid, bss->ssid, 32);
    }

    if (bss->channel != 0) {
        struct router_info *info = NULL;

        os_printf("ssid %s, channel %d, authmode %d, rssi %d\n",
        ssid, bss->channel, bss->authmode, bss->rssi);
        channel_bits |= 1 << (bss->channel);

        info = (struct router_info *)os_zalloc(sizeof(struct router_info));
        info->authmode = bss->authmode;
        info->channel = bss->channel;
        os_memcpy(info->bssid, bss->bssid, 6);

        SLIST_INSERT_HEAD(&router_list, info, next);
    }
        bss = STAILQ_NEXT(bss, next);
    }

    for (i = current_channel; i < 14; i++) {
        if ((channel_bits & (1 << i)) != 0) {
            current_channel = i + 1;
            wifi_set_channel(i);
            os_printf("current channel1 %d--------------------------------------------%d\n", i, system_get_time());
            break;
        }
    }

    wifi_promiscuous_enable(1);
    wifi_set_promiscuous_rx_cb(wifi_promiscuous_rx);

    os_timer_disarm(&channel_timer);
    os_timer_setfn(&channel_timer, channel_timer_cb, NULL);
    os_timer_arm(&channel_timer, SCAN_TIMEINTERVAL, 0);

    } else {
        os_printf("err, scan status %d\n", status);
    }
}

void ICACHE_FLASH_ATTR
system_init_done(void)
{
    SLIST_INIT(&router_list);

    wifi_station_scan(NULL,wifi_scan_done);
}


void user_init(void)
{
    os_printf("smart connection OR smart link demo v 1001\n"

    wifi_set_opmode(STATION_MODE);

    system_init_done_cb(system_init_done);
}
