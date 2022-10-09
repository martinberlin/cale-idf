/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once
#include <esp_err.h>
#include <esp_event.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ESP RainMaker Event Base */
ESP_EVENT_DECLARE_BASE(APP_WIFI_EVENT);

/** App Wi-Fir Events */
typedef enum {
    /** QR code available for display. Associated data is the NULL terminated QR payload. */
    APP_WIFI_EVENT_QR_DISPLAY = 1,
    /** Provisioning timed out */
    APP_WIFI_EVENT_PROV_TIMEOUT,
    /** Provisioning has restarted due to failures (Invalid SSID/Passphrase) */
    APP_WIFI_EVENT_PROV_RESTART,
} app_wifi_event_t;

/** Types of Proof of Possession */
typedef enum {
    /** Use MAC address to generate PoP */
    POP_TYPE_MAC,
    /** Use random stream generated and stored in fctry partition during claiming process as PoP */
    POP_TYPE_RANDOM,
    /** Do not use any PoP.
     * Use this option with caution. Consider using `CONFIG_APP_WIFI_PROV_TIMEOUT_PERIOD` with this.
     */
    POP_TYPE_NONE
} app_wifi_pop_type_t;

void app_wifi_init();
esp_err_t app_wifi_start(app_wifi_pop_type_t pop_type);

#ifdef __cplusplus
}
#endif
