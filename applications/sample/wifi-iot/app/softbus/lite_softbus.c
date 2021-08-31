#include "ohos_init.h"
#include "cmsis_os2.h"
//#include "wifi_connecter.h"
#include "wifi_device.h"

#include "discovery_service.h"
#include "session.h"
#include "coap_discover.h"
#include "tcp_session_manager.h"
#include "nstackx.h"

#include <stdio.h>
#include <string.h>

void onSuccess(int publishId);
void onFail(int publishId, PublishFailReason reason);
void wifiConnect(void);

//定义参数
static PublishInfo info=
{
    .publishId=1,
    .mode=0x55,//DISCOVER_MODE_PASSIVE
    .medium=COAP,
    .freq=HIGH,
    .capability="ddmpCapability",
    .capabilityData=(unsigned char *)"123",
    .dataLen=2
};

static IPublishCallback cb=
{
    .onPublishSuccess = onSuccess,
    .onPublishFail = onFail
};

void onSuccess(int publishId)
{
    printf("publish succeeded, publishId = %d\r\n", publishId);
}

void onFail(int publishId, PublishFailReason reason)
{
    printf("publish failed, publishId = %d, reason = %d\r\n", publishId, reason);
}


void liteSoftBusTask(void)
{
    //WifiConnect();
    osDelay(100);
    printf("enter SoftBus Task\r\n");
    int ret = PublishService("czy", &info, &cb);
    if(ret!=0)
    {
        printf("PublishService init failed\n");
    }
    else
    {
        printf("PublishService init success\n");
    }
}

//SYS_RUN(liteSoftBusTask);
//init一定要用系统服务层！！！！！！！！！！！！！！！！不然会发生未知错误，卡了一万年fuck！！！！！！
SYSEX_SERVICE_INIT(liteSoftBusTask);