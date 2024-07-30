/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights
 * reserved. PROPRIETARY/CONFIDENTIAL. USE IS SUBJECT TO LICENSE TERMS.
 */
// Include logging macros
#include <orpheus/framework/utils/logging/Log.h>

// Include Auto generated ModeManager Service IPC skeleton
#include <ModeManagerServiceSkeleton.h>

// Include ModeManager Service related headers
#include "ModeManagerServiceApp.h"
#include "ModeManagerService.h"

// Log tag for this file
DEFINE_LOG_TAG("ModeManagerService");

namespace orpheus::service::ModeManagerService
{

    // Allow the use of short names for types.
    using orpheus::service::ModeManagerService::ModeManagerService;
    using orpheus::service::ModeManagerService::ModeManagerServiceSkeleton;
    namespace log = orpheus::framework::log;

    /**
     * @brief Lifecycle method called when service is being started
     *
     * @return int
     */
    int ModeManagerServiceApp::onStart()
    {
        LOG_F(INFO, "{} invoked", __FUNCTION__);

        // Configure logger verbosity
        log::Options loggerOptions{};
        loggerOptions.platformVerbosity = log::VERBOSITY_INFO;
        log::init(loggerOptions);

        // Set the ModeManager Service IPC APIs handling object
        ModeManagerServiceSkeleton::getInstance().setService(&ModeManagerService::getInstance());

        // Set the ModeManager Service Connection events handling object
        ModeManagerServiceSkeleton::getInstance().setConnectionEventDelegate(
            &ModeManagerService::getInstance());

        return 0;
    }

    /**
     * @brief Lifecycle method called when service is being stopped
     *
     * @return int
     */
    int ModeManagerServiceApp::onStop()
    {
        LOG_F(INFO, "{} invoked", __FUNCTION__);
        return 0;
    }
} // namespace orpheus::service::ModeManagerService