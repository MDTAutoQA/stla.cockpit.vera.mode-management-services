#define PUBLIC __attribute__((visibility("default")))
// Include LCM headers
#include <orpheus/framework/app/Application.h>
#include <orpheus/framework/lifecycle-manager/LcmClient.h>

// Include logging macros
#include <orpheus/framework/utils/logging/Log.h>

// Include the ModeManager Service Application class
#include "ModeManagerServiceApp.h"

// Log tag for this file
DEFINE_LOG_TAG("ModeManagerService");

// Allow the use of short names for types.
using orpheus::framework::app::BaseApplication;
using orpheus::framework::lifecycleManager::LcmClient;
using orpheus::service::ModeManagerService::ModeManagerServiceApp;

/**
 * @brief Factory method to create the ModeManager Service instance
 *
 * @param appId
 * @param c
 * @return std::unique_ptr<BaseApplication>
 */
static std::unique_ptr<BaseApplication> ModeManagerServiceAppFactory(
    std::string_view appId,
    orpheus::framework::app::ApplicationContext &c)
{
    return std::make_unique<ModeManagerServiceApp>(c);
}

extern "C"
{
    /**
     * @brief  Called by LCM on load of the ModeManager application
     *
     * @return int
     */
    PUBLIC int onLoad()
    {
        LOG_F(INFO, "Starting ModeManager");

        // Set the factory for the ModeManager service
        LcmClient &client{LcmClient::getInstance()};
        int const status{client.addAppClassFactory("com.stellantis.service.modemanager.main", ModeManagerServiceAppFactory)};
        if (status != 0)
        {
            LOG_F(ERROR, "Failed to add ModeManager service factory: %d", status);
        }

        return 0;
    }
}
