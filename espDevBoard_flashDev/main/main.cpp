#include "consts.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "stdint.h"

#define NVS_TAG     "NVS"
#define NVS_NAMESPACE  "nvsStorage"

extern "C" {
    void app_main();
}

namespace intermode {

namespace nvs {

/*
 * Abstract base nonvolative storage hardware abstraction layer class
 */
class Abstract_NVS_HalTypeDef {
    protected:
        constexpr static const char *kpcTag = "NVS";        // Tag to use for logging messages
    public:
        /**
         * @brief Mode of opening the non-volatile storage
         */
        typedef enum {
            READONLY,           /*!< Read only */
            READWRITE           /*!< Read and write */
        } OpenMode;

        virtual ReturnCodes eInitialize(void) = 0;          // Initialize NVS
        virtual ReturnCodes eErase(const char* = NULL) = 0; // Erase NVS
        virtual ReturnCodes eOpen(const char*, OpenMode, uint32_t*, const char* = NULL) = 0;    // Open NVS
        virtual ReturnCodes eClose(uint32_t) = 0;           // Close NVS
        // TODO: Switch from overloading to templates
        virtual ReturnCodes eRead(uint32_t, const char*, int8_t*) = 0;      // Read from NVS
        virtual ReturnCodes eWrite(uint32_t, const char*, int8_t) = 0;      // Write to NVS
        virtual ReturnCodes eRead(uint32_t, const char*, int32_t*) = 0;     // Read from NVS
        virtual ReturnCodes eWrite(uint32_t, const char*, int32_t) = 0;     // Write to NVS
        virtual ReturnCodes eCommit(uint32_t) = 0;          // Commit NVS
};

/*
 * ESP32 nonvolative storage hardware abstraction layer class
 */
class ESP_NVS_HalTypeDef : public Abstract_NVS_HalTypeDef {
    public:
        ReturnCodes eInitialize(void) override;             // Initialize NVS
        ReturnCodes eErase(const char* = NULL) override;    // Erase NVS
        ReturnCodes eOpen(const char*, OpenMode, uint32_t*, const char* = NULL) override;       // Open NVS
        ReturnCodes eClose(uint32_t) override;              // Close NVS
        ReturnCodes eRead(uint32_t, const char*, int8_t*) override;         // Read from NVS
        ReturnCodes eWrite(uint32_t, const char*, int8_t) override;         // Write to NVS
        ReturnCodes eRead(uint32_t, const char*, int32_t*) override;        // Read from NVS
        ReturnCodes eWrite(uint32_t, const char*, int32_t) override;        // Write to NVS
        ReturnCodes eCommit(uint32_t) override;             // Commit NVS

    private:
        ReturnCodes eLogRead(esp_err_t, const char*);       // Print log messages for a read operation
        ReturnCodes eLogWrite(esp_err_t, const char*);      // Print log messages for a write operation
};



/*
 * Initialize nonvolatile storage
 *
 * @return  Return code
 * @retval  keReturnNormal      Initialization successful
 * @retval  keReturnFull        No free pages
 * @retval  keReturnVersion     Incompatible NVS version found
 * @retval  keReturnNotFound    No NVS partition found
 * @retval  keReturnMemory      Memory could not be allocated
 */
ReturnCodes ESP_NVS_HalTypeDef::eInitialize(void) {
    ReturnCodes eReturnCode = keReturnNormal;
    esp_err_t xErr = nvs_flash_init();

    switch(xErr) {
        case ESP_OK:
            ESP_LOGI(this->kpcTag, "NVS initialized");
            break;
        case ESP_ERR_NVS_NO_FREE_PAGES:
            ESP_LOGE(this->kpcTag, "%s No free pages", esp_err_to_name(xErr));
            eReturnCode = keReturnFull;
            break;
        case ESP_ERR_NVS_NEW_VERSION_FOUND:
            ESP_LOGE(this->kpcTag, "%s Incompatible NVS version found", esp_err_to_name(xErr));
            eReturnCode = keReturnVersion;
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE(this->kpcTag, "%s No NVS partition found", esp_err_to_name(xErr));
            eReturnCode = keReturnPartitionNotFound;
            break;
        case ESP_ERR_NO_MEM:
            ESP_LOGE(this->kpcTag, "%s Memory could not be allocated", esp_err_to_name(xErr));
            eReturnCode = keReturnMemory;
            break;
        default:
            ESP_LOGE(this->kpcTag, "%s NVS initialization failed", esp_err_to_name(xErr));
            eReturnCode = keReturnError;
            break;
    }

    return eReturnCode;
}

/*
 * Erase nonvolatile storage
 *
 * @param   kpcPartition        Partition to erase
 * 
 * @return  Return code
 * @retval  keReturnNormal      Erase successful
 * @retval  keReturnNotFound    No NVS partition found
 * @retval  keReturnError       Erase failed
 */
ReturnCodes ESP_NVS_HalTypeDef::eErase(const char* kpcPartition) {
    ReturnCodes eReturnCode = keReturnNormal;
    esp_err_t xErr = ESP_OK;

    if(NULL == kpcPartition) {
        xErr = nvs_flash_erase();
    } else {
        xErr = nvs_flash_erase_partition(kpcPartition);
    }

    switch(xErr) {
        case ESP_OK:
            ESP_LOGI(this->kpcTag, "NVS erased");
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE(this->kpcTag, "%s NVS partition not found", esp_err_to_name(xErr));
            eReturnCode = keReturnPartitionNotFound;
            break;
        default:
            ESP_LOGE(this->kpcTag, "%s NVS erase failed", esp_err_to_name(xErr));
            eReturnCode = keReturnError;
            break;
    }

    return eReturnCode;
}

/*
 * Open nonvolatile storage
 *
 * @param   kpcNamespace        Namespace to open
 * @param   eMode               Mode to open the partition in
 * @param   pulHandle           Pointer to handle to store
 * @param   kpcPartition        Partition to open
 * 
 * @return  Return code
 * @retval  keReturnNormal              Erase successful
 * @retval  keReturnNotInitialized      NVS not initialized
 * @retval  keReturnPartitionNotFound   NVS partition not found
 * @retval  keReturnNamespaceNotFound   NVS namespace not found
 * @retval  keReturnName                Invalid NVS namespace name
 * @retval  keReturnMemory              Memory could not be allocated
 * @retval  keReturnFull                Not enough space
 * @retval  keReturnError               Erase failed
 */
ReturnCodes ESP_NVS_HalTypeDef::eOpen(const char* kpcNamespace, OpenMode eMode, uint32_t* pulHandle, const char* kpcPartition) {
    ReturnCodes eReturnCode = keReturnNormal;
    esp_err_t xErr = ESP_OK;
    nvs_open_mode_t xMode = (eMode == READONLY) ? NVS_READONLY : NVS_READWRITE;

    if(NULL == kpcPartition) {
        xErr = nvs_open(kpcNamespace, xMode, pulHandle);
    } else {
        xErr = nvs_open_from_partition(kpcPartition, kpcNamespace, xMode, pulHandle);
    }

    switch(xErr) {
        case ESP_OK:
            ESP_LOGI(this->kpcTag, "NVS opened");
            break;
        case ESP_ERR_NVS_NOT_INITIALIZED:
            ESP_LOGE(this->kpcTag, "%s NVS not initialized", esp_err_to_name(xErr));
            eReturnCode = keReturnNotInitialized;
            break;
        case ESP_ERR_NVS_PART_NOT_FOUND:
            ESP_LOGE(this->kpcTag, "%s NVS partition not found", esp_err_to_name(xErr));
            eReturnCode = keReturnPartitionNotFound;
            break;
        case ESP_ERR_NVS_NOT_FOUND :
            ESP_LOGE(this->kpcTag, "%s NVS namespace %s not found", esp_err_to_name(xErr), kpcNamespace);
            eReturnCode = keReturnNamespaceNotFound;
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(this->kpcTag, "%s Invalid NVS name %s", esp_err_to_name(xErr), kpcNamespace);
            eReturnCode = keReturnName;
            break;
        case ESP_ERR_NO_MEM:
            ESP_LOGE(this->kpcTag, "%s Memory could not be allocated", esp_err_to_name(xErr));
            eReturnCode = keReturnMemory;
            break;
        case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
            ESP_LOGE(this->kpcTag, "%s Not enough space", esp_err_to_name(xErr));
            eReturnCode = keReturnFull;
            break;
        default:
            ESP_LOGE(this->kpcTag, "%s NVS open failed", esp_err_to_name(xErr));
            eReturnCode = keReturnError;
            break;
    }

    return eReturnCode;
}

/*
 * Close nonvolatile storage
 *
 * @param   ulHandle            Handle to close
 */
ReturnCodes ESP_NVS_HalTypeDef::eClose(uint32_t ulHandle) {
    nvs_close(ulHandle);
    ESP_LOGI(this->kpcTag, "NVS closed");
    return keReturnNormal;
}

/*
 * Read from nonvolatile storage
 *
 * @param   ulHandle            Handle to read from
 * @param   kpcKey              Key to read
 * @param   pcValue             Location to store read value
 * 
 * @return  Return code
 * @retval  keReturnNormal      Read successful
 * @retval  keReturnNotFound    Key not found
 * @retval  keReturnInvalid     Invalid handle
 * @retval  keReturnName        Invalid key name
 * @retval  keReturnSize        Insufficient storage space
 * @retval  keReturnError       Read failed
 */
ReturnCodes ESP_NVS_HalTypeDef::eRead(uint32_t ulHandle, const char* kpcKey, int8_t* pcValue) {
    esp_err_t xErr = nvs_get_i8(ulHandle, kpcKey, pcValue);
    ReturnCodes eReturnCode = this->eLogRead(xErr, kpcKey);

    if(keReturnNormal == eReturnCode) {
        ESP_LOGI(this->kpcTag, "Read %s = %d", kpcKey, *pcValue);
    }

    return eReturnCode;
}

/*
 * Write to nonvolatile storage
 *
 * @param   ulHandle            Handle to write to
 * @param   kpcKey              Key to write to
 * @param   cValue              Value to write
 * 
 * @return  Return code
 * @retval  keReturnNormal      Write successful
 * @retval  keReturnInvalid     Invalid handle
 * @retval  keReturnPermission  Read-only partition
 * @retval  keReturnName        Invalid key name
 * @retval  keReturnSize        Insufficient storage space
 * @retval  keReturnReinit      NVS needs to be reinitialized
 * @retval  keReturnError       Write failed
 */
ReturnCodes ESP_NVS_HalTypeDef::eWrite(uint32_t ulHandle, const char* kpcKey, int8_t cValue) {
    esp_err_t xErr = nvs_set_i8(ulHandle, kpcKey, cValue);
    ReturnCodes eReturnCode = this->eLogWrite(xErr, kpcKey);

    if(keReturnNormal == eReturnCode) {
            ESP_LOGI(this->kpcTag, "Wrote %s = %d", kpcKey, cValue);
    }

    return eReturnCode;
}

/*
 * Read from nonvolatile storage
 *
 * @param   ulHandle            Handle to read from
 * @param   kpcKey              Key to read
 * @param   plValue             Location to store read value
 * 
 * @return  Return code
 * @retval  keReturnNormal      Read successful
 * @retval  keReturnNotFound    Key not found
 * @retval  keReturnInvalid     Invalid handle
 * @retval  keReturnName        Invalid key name
 * @retval  keReturnSize        Insufficient storage space
 * @retval  keReturnError       Read failed
 */
ReturnCodes ESP_NVS_HalTypeDef::eRead(uint32_t ulHandle, const char* kpcKey, int32_t* plValue) {
    esp_err_t xErr = nvs_get_i32(ulHandle, kpcKey, plValue);
    ReturnCodes eReturnCode = this->eLogRead(xErr, kpcKey);

    if(keReturnNormal == eReturnCode) {
        ESP_LOGI(this->kpcTag, "Read %s = %d", kpcKey, *plValue);
    }

    return eReturnCode;
}

/*
 * Write to nonvolatile storage
 *
 * @param   ulHandle            Handle to write to
 * @param   kpcKey              Key to write to
 * @param   lValue              Value to write
 * 
 * @return  Return code
 * @retval  keReturnNormal      Write successful
 * @retval  keReturnInvalid     Invalid handle
 * @retval  keReturnPermission  Read-only partition
 * @retval  keReturnName        Invalid key name
 * @retval  keReturnSize        Insufficient storage space
 * @retval  keReturnReinit      NVS needs to be reinitialized
 * @retval  keReturnError       Write failed
 */
ReturnCodes ESP_NVS_HalTypeDef::eWrite(uint32_t ulHandle, const char* kpcKey, int32_t lValue) {
    esp_err_t xErr = nvs_set_i32(ulHandle, kpcKey, lValue);
    ReturnCodes eReturnCode = this->eLogWrite(xErr, kpcKey);

    if(keReturnNormal == eReturnCode) {
            ESP_LOGI(this->kpcTag, "Wrote %s = %d", kpcKey, lValue);
    }

    return eReturnCode;
}

/*
 * Commit pending changes to nonvolatile storage
 *
 * @param   ulHandle            Handle to commit
 * 
 * @return  Return code
 * @retval  keReturnNormal              Commit successful
 * @retval  keReturnInvalid             Invalid NVS handle
 * @retval  keReturnError               Commit failed
 */
ReturnCodes ESP_NVS_HalTypeDef::eCommit(uint32_t ulHandle) {
    ReturnCodes eReturnCode = keReturnNormal;
    esp_err_t xErr = nvs_commit(ulHandle);
    
    switch(xErr) {
        case ESP_OK:
            ESP_LOGI(this->kpcTag, "Changes committed");
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(this->kpcTag, "%s Invalid partition", esp_err_to_name(xErr));
            eReturnCode = keReturnInvalid;
            break;
        default:
            ESP_LOGE(this->kpcTag, "%s NVS commit failed", esp_err_to_name(xErr));
            eReturnCode = keReturnError;
            break;
    }

    return eReturnCode;
}

/*
 * Print log messages for a read operation
 *
 * @param   xCode               Return code from read operation
 * @param   kpcKey              Key read
 */
ReturnCodes ESP_NVS_HalTypeDef::eLogRead(esp_err_t xCode, const char* kpcKey) {
    ReturnCodes eReturnCode = keReturnNormal;

    switch(xCode) {
        case ESP_OK:
            ESP_LOGI(this->kpcTag, "Read successful");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(this->kpcTag, "%s Key %s not found", esp_err_to_name(xCode), kpcKey);
            eReturnCode = keReturnNotFound;
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(this->kpcTag, "%s Invalid partition", esp_err_to_name(xCode));
            eReturnCode = keReturnInvalid;
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(this->kpcTag, "%s Invalid key %s", esp_err_to_name(xCode), kpcKey);
            eReturnCode = keReturnName;
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(this->kpcTag, "%s Not enough space to store value", esp_err_to_name(xCode));
            eReturnCode = keReturnSize;
            break;
        default:
            ESP_LOGE(this->kpcTag, "%s Read failed", esp_err_to_name(xCode));
            eReturnCode = keReturnError;
            break;
    }

    return eReturnCode;
}

/*
 * Print log messages for a write operation
 *
 * @param   xCode               Return code from write operation
 * @param   kpcKey              Key written to
 */
ReturnCodes ESP_NVS_HalTypeDef::eLogWrite(esp_err_t xCode, const char* kpcKey) {
    ReturnCodes eReturnCode = keReturnNormal;

    switch(xCode) {
        case ESP_OK:
            ESP_LOGI(this->kpcTag, "Write successful");
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(this->kpcTag, "%s Invalid partition", esp_err_to_name(xCode));
            eReturnCode = keReturnInvalid;
            break;
        case ESP_ERR_NVS_READ_ONLY:
            ESP_LOGE(this->kpcTag, "%s Partition configured as read-only", esp_err_to_name(xCode));
            eReturnCode = keReturnPermission;
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(this->kpcTag, "%s Invalid key %s", esp_err_to_name(xCode), kpcKey);
            eReturnCode = keReturnName;
            break;
        case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
            ESP_LOGE(this->kpcTag, "%s Not enough space to store value", esp_err_to_name(xCode));
            eReturnCode = keReturnSize;
            break;
        case ESP_ERR_NVS_REMOVE_FAILED:
            ESP_LOGE(this->kpcTag, "%s NVS needs to be reinitialized", esp_err_to_name(xCode));
            eReturnCode = keReturnReinit;
            break;
        default:
            ESP_LOGE(this->kpcTag, "%s Write failed", esp_err_to_name(xCode));
            eReturnCode = keReturnError;
            break;
    }

    return eReturnCode;
}

}   // namespace nvs

}   // namespace intermode


using namespace intermode;
using namespace intermode::nvs;
void app_main(void)
{
    // Initialize NVS
    ESP_NVS_HalTypeDef xNvsHal;
    ReturnCodes xErr = xNvsHal.eInitialize();
    esp_err_t err = ESP_OK;
    if (xErr == keReturnFull || xErr == keReturnVersion) {
        // NVS partition has no free space (possibly due to truncation) or the
        //  partition was previously used by a different version of NVS.
        // Erase and then reattempt nvs initialization.
        ESP_LOGE(NVS_TAG, "Error initializing NVS, erasing and retrying");

        xErr = xNvsHal.eErase();
        if(xErr != keReturnNormal) {
            abort();
        }

        xErr = xNvsHal.eInitialize();
    }
    if(xErr != keReturnNormal) {
        abort();
    }

    uint32_t my_handle;
    xErr = xNvsHal.eOpen(NVS_NAMESPACE, xNvsHal.READWRITE, &my_handle);

    if (xErr == keReturnNormal) {
         // Read
        ESP_LOGI(NVS_TAG, "Reading restart counter from NVS ... ");
        int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = xNvsHal.eRead(my_handle, "restart_counter", &restart_counter);

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
        xErr = xNvsHal.eWrite(my_handle, "restart_counter", restart_counter);
        if(err != keReturnNormal) {
            ESP_LOGE(NVS_TAG, "Error (%s) writing!", esp_err_to_name(err));
        } else {
            ESP_LOGI(NVS_TAG, "Done");
        }

        xNvsHal.eCommit(my_handle);
        xNvsHal.eClose(my_handle);
    }

    

    uint32_t my_handle_2;
    xErr = xNvsHal.eOpen("nvsStorage2", xNvsHal.READWRITE, &my_handle_2);

    if (xErr == keReturnNormal) {
         // Read
        ESP_LOGI(NVS_TAG, "Reading restart counter from NVS ... ");
        int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = xNvsHal.eRead(my_handle_2, "restart_counter", &restart_counter);

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
        xErr = xNvsHal.eWrite(my_handle_2, "restart_counter", restart_counter);
        if(err != keReturnNormal) {
            ESP_LOGE(NVS_TAG, "Error (%s) writing!", esp_err_to_name(err));
        } else {
            ESP_LOGI(NVS_TAG, "Done");
        }

        xNvsHal.eCommit(my_handle_2);
        xNvsHal.eClose(my_handle_2);
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
