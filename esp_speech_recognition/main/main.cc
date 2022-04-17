/*
 * @Author: HoGC
 * @Date: 2022-04-17 21:56:31
 * @Last Modified time: 2022-04-17 21:56:31
 */
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "esp_netif.h"
#include "nvs_flash.h"

#include "app_speech.h"

#include "app_wifi.h"


extern "C" void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    app_wifi_init();
    app_wifi_connect("HoGC", "227227227");

    app_speech_init();
}
