#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_connecter.h"

#include "discovery_service.h"
#include "session.h"
#include "coap_discover.h"
#include "tcp_session_manager.h"
#include "nstackx.h"

#include <stdio.h>
#include <string.h>

// 定义业务⾃身的业务名称，会话名称及相关回调
const char *g_pkgName = "BUSINESS_NAME";
const char *g_sessionName = "SESSION_NAME";
const char *g_demoModuleName = "czy";
struct ISessionListener *g_sessionCallback = NULL;

#define NAME_LENGTH 64
#define TRANS_FAILED -1

// 回调实现：接收对方通过SendBytes发送的数据，此示例实现是接收到对端发送的数据后回复固定消息
void OnBytesReceivedTest(int sessionId, const void *data, unsigned int dataLen)
{
    printf("OnBytesReceivedTest\n");
    printf("Recv Data: %s\n", (char *)data);
    printf("Recv Data dataLen: %d\n", dataLen);
    char *testSendData = "Hello World, Hello!";
    SendBytes(sessionId, testSendData, strlen(testSendData));
    return;
}

// 回调实现：用于处理会话关闭后的相关业务操作，如释放当前会话相关的业务资源，会话无需业务主动释放
void OnSessionClosedEventTest(int sessionId)
{
    printf("Close session successfully, sessionId=%d\n", sessionId);
}

// 回调实现：用于处理会话打开后的相关业务操作。返回值为0，表示接收；反之，非0表示拒绝。此示例表示只接受其他设备的同名会话连接
int OnSessionOpenedEventTest(int sessionId)
{
    char sessionNameBuffer[NAME_LENGTH + 1];
    if (GetPeerSessionName(sessionId, sessionNameBuffer, NAME_LENGTH) == TRANS_FAILED)
    {
        printf("GetPeerSessionName faild, which sessionId = %d\n", sessionId);
        return -1;
    }
    if (strcmp(sessionNameBuffer, g_sessionName) != 0)
    {
        printf("Reject the session which name is different from mine, sessionId=%d\n", sessionId);
        return -1;
    }
    printf("Open session successfully, sessionId=%d\n", sessionId);
    return 0;
}

// 向SoftBus注册业务会话服务及其回调
int StartSessionServer()
{
    if (g_sessionCallback == NULL)
    {
        g_sessionCallback = (struct ISessionListener *)malloc(sizeof(struct ISessionListener));
    }
    if (g_sessionCallback == NULL)
    {
        printf("Failed to malloc g_sessionCallback!\n");
        return -1;
    }
    g_sessionCallback->onBytesReceived = OnBytesReceivedTest;
    g_sessionCallback->onSessionOpened = OnSessionOpenedEventTest;
    g_sessionCallback->onSessionClosed = OnSessionClosedEventTest;
    int ret = CreateSessionServer(g_pkgName, g_sessionName, g_sessionCallback);
    if (ret < 0)
    {
        printf("Failed to create session server!\n");
        free(g_sessionCallback);
        g_sessionCallback = NULL;
    }
    return ret;
}

// 从SoftBus中删除业务会话服务及其回调
void StopSessionServer()
{
    int ret = RemoveSessionServer(g_pkgName, g_sessionName);
    if (ret < 0)
    {
        printf("Failed to remove session server!\n");
        return;
    }
    if (g_sessionCallback != NULL)
    {
        free(g_sessionCallback);
        g_sessionCallback = NULL;
    }
}

// 回调函数声明：
void onSuccess(int publishId)
{
    printf("publish succeeded, publishId = %d\r\n", publishId);
    char ipbuff[NSTACKX_MAX_IP_STRING_LEN] = {"0.0.0.0"};
    CoapGetIp(ipbuff, NSTACKX_MAX_IP_STRING_LEN, 0);
    printf("CoapGetIp = %s\n", ipbuff);
    if (StartSessionServer() != -1)
        printf("StartSessionServer successed!\n");
}
void onFail(int publishId, PublishFailReason reason)
{
    printf("publish failed, publishId = %d, reason = %d\r\n", publishId, reason);
}
void wifiConnect()
{
    WifiDeviceConfig apConfig = {0};
    strcpy(apConfig.ssid, "CZH");
    strcpy(apConfig.preSharedKey, "CZHCZYZX");
    apConfig.securityType = WIFI_SEC_TYPE_PSK;
    int netId = ConnectToHotspot(&apConfig);
}
static PublishInfo g_publishInfo = {
    .capabilityData = (unsigned char *)"1",
    .capability = "czyTest",
    .dataLen = 1,
    .publishId = 1,
    .mode = DISCOVER_MODE_PASSIVE,
    .medium = COAP,
    .freq = MID,
};
static IPublishCallback g_publishCallback = {
    .onPublishSuccess = onSuccess,
    .onPublishFail = onFail,
};

void softbus_task(void *arg)
{
    printf("start enter softbus task\n");
    wifiConnect();
    (void)arg;
    // 服务发布接口使用

    int ret = PublishService(g_demoModuleName, &g_publishInfo, &g_publishCallback);
    if(ret!=0)
    {
        printf("PublishService init failed\n");
    }
    printf("PublishService init success\n");
}

void SoftBusDemo(void)
{
    osThreadAttr_t attr;
    printf("start create SoftBusTask\n");
    attr.name = "SoftBusTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(softbus_task, NULL, &attr) == NULL)
    {
        printf("[SoftBustDemo] Falied to create softbus_task!\n");
    }
}

SYS_RUN(SoftBusDemo);