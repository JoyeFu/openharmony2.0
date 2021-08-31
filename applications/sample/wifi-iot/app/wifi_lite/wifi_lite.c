#include "wifi_device.h"
#include "ohos_init.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char ssid[WIFI_MAX_SSID_LEN] = "CZH";             //WLAN的名称
static unsigned char bssid[WIFI_MAC_LEN];                //findWifiBssid自动获取，也可手动输入
static unsigned char key[WIFI_MAX_KEY_LEN] = "CZHCZYZX"; //待链接wifi密码
WifiScanInfo *infoList;
unsigned int size = WIFI_SCAN_HOTSPOT_LIMIT;

void wifiStart(void)
{
    WifiErrorCode error = EnableWifi();
    if (error != WIFI_SUCCESS)
    {
        printf("[wifi] start failed\r\n");
        return;
    }
    printf("[wifi] start success\r\n");
}

void findWifiBssid(void)
{
    unsigned int i = 0;
    for (; i < size; ++i)
    {
        if (!strcmp(infoList[i].ssid, ssid))
        {
            printf("Bssid = %s\r\n", infoList[i].bssid);
        }
    }
}

void wifiScan(void)
{
    infoList=malloc(sizeof(WifiScanInfo) * WIFI_SCAN_HOTSPOT_LIMIT);//不能放在外面malloc
    WifiErrorCode error = Scan();
    if (error != WIFI_SUCCESS)
    {
        printf("[wifi] scan failed\r\n");
        return;
    }
    error = GetScanInfoList(infoList, &size);
    if (error != WIFI_SUCCESS || size == 0)
    {
        printf("[wifi] scan get scan info list failed\r\n");
        return;
    }
    findWifiBssid();
}

void wifiConnect(void)
{
    wifiStart();
    wifiScan();
    int netId = 0;
    WifiDeviceConfig config = {0};
    config.freq = 20;
    config.securityType = WIFI_SEC_TYPE_PSK;
    config.wapiPskType = WIFI_PSK_TYPE_ASCII;
    memcpy_s(config.ssid, WIFI_MAX_SSID_LEN, ssid, strlen(ssid));
    memcpy_s(config.bssid, WIFI_MAC_LEN, bssid, WIFI_MAC_LEN);
    memcpy_s(config.preSharedKey, WIFI_MAX_KEY_LEN, key, strlen(key));
    WifiErrorCode error = AddDeviceConfig(&config, &netId);
    if (error != WIFI_SUCCESS || netId == 0)
    {
        printf("[wifi] config failed\r\n");
        return;
    }
    error = ConnectTo(netId);
    if (error != WIFI_SUCCESS)
    {
        printf("[wifi] connect failed\r\n");
        return;
    }
    printf("[wifi] connect success\r\n");
}

SYS_RUN(wifiConnect);