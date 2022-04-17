/*
 * @Author: HoGC
 * @Date: 2022-04-16 13:32:19
 * @Last Modified time: 2022-04-16 13:32:19
 */
#ifndef __APP_WIFI_H__
#define __APP_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

bool app_wifi_get_connect_status(void);
void app_wifi_connect(const char *ssid, const char *password);

void app_wifi_smartconfig_start(void);

void app_wifi_init(void);

#ifdef __cplusplus
}
#endif

#endif // __APP_WIFI_H__

