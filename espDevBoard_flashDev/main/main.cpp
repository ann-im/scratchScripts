#include "consts.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#define NVS_TAG     "NVS"
#define NVS_HANDLE  "nvsStorage"

extern "C" {
    void app_main();
}

namespace intermode {

namespace nvs {

/*
 * Abstract base non-volative storage hardware abstraction layer class
 */
class Abstract_NVS_HalTypeDef {
    public:
        virtual 
};

/*
 * ESP32 Non-Volative Storage hardware abstraction layer class
 */
class ESP_NVS_HalTypeDef : public Abstract_NVS_HalTypeDef {

};

}   // namespace nvs

}   // namespace intermode

void app_main(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition has no free space (possibly due to truncation) or the
        //  partition was previously used by a different version of NVS.
        // Erase and then reattempt nvs initialization.
        ESP_LOGE(NVS_TAG, "Error (%s) initializing NVS, erasing and retrying", esp_err_to_name(err));
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    ESP_LOGI(NVS_TAG, "NVS initialized");

    nvs_handle_t my_handle;
    err = nvs_open(NVS_HANDLE, NVS_READWRITE, &my_handle);

    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } else {
        ESP_LOGI(NVS_TAG, "NVS handle opened successfully");

        // Read
        ESP_LOGI(NVS_TAG, "Reading restart counter from NVS ... ");
        int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(my_handle, "restart_counter", &restart_counter);

        switch (err) {
            case ESP_OK:
                ESP_LOGI(NVS_TAG, "Done");
                ESP_LOGI(NVS_TAG, "Restart counter = %d", restart_counter);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGE(NVS_TAG, "Key does not exist");
                break;
            default :
                ESP_LOGE(NVS_TAG, "Error (%s) reading!", esp_err_to_name(err));
        }

        // Write
        ESP_LOGI(NVS_TAG, "Updating restart counter in NVS ... ");
        restart_counter++;
        err = nvs_set_i32(my_handle, "restart_counter", restart_counter);
        if(err != ESP_OK) {
            ESP_LOGE(NVS_TAG, "Error (%s) writing!", esp_err_to_name(err));
        } else {
            ESP_LOGI(NVS_TAG, "Done");
        }

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        ESP_LOGI(NVS_TAG, "Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        if(err != ESP_OK) {
            ESP_LOGE(NVS_TAG, "Error (%s) writing!", esp_err_to_name(err));
        } else {
            ESP_LOGI(NVS_TAG, "Done");
        }

        // Close
        nvs_close(my_handle);
    }

    // Restart module
    for (int i = 10; i >= 0; i--) {
        ESP_LOGI(NVS_TAG, "Restarting in %d seconds...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(NVS_TAG, "Restarting now.");
    fflush(stdout);
    esp_restart();
}
