/*
 * Copyright (C) 2021 Stellantis/MobileDrive and/or their affiliated companies. All rights reserved.
 *
 * This software, including documentation, is protected by copyright and controlled by
 * Stellantis/MobileDrive. All rights are reserved. Copying, including reproducing, storing,
 * adapting or translating any or all of this material requires the prior written
 * consent of Stellantis/MobileDrive jointly. This material also contains confidential information,
 * which may not be disclosed to others without the prior and joint written consent of Stellantis/MobileDrive.
 *
 * ****** About File **********
 * contains the LCM service application logic.
 */

// Include logging
#include <orpheus/framework/utils/logging/Log.h>

// Include ModeManager Service headers
#include "ModeManagerService.h"
#include <ModeManagerServiceSkeleton.h>

#include "Utility.h"

// Log tag for this file
DEFINE_LOG_TAG("ModeManagerService");

using std::string;

namespace orpheus::service::ModeManagerService
{
    using apmf::ApmfBase;
    using apmf::GetProcessObject;
    using apmf::Ptr;
    using apmf::iface::com::amazon::apmf::BaseError;
    using apmf::iface::com::amazon::apmf::IProcess;
    using apmf::iface::com::stellantis::projectionIface::androidAuto::IAndroidAutoClient;
    using apmf::iface::com::stellantis::projectionIface::androidAuto::IAndroidAutoClientFactory;
    using apmf::iface::com::stellantis::projectionIface::carplay::ICarPlayClient;
    using apmf::iface::com::stellantis::projectionIface::carplay::ICarPlayClientFactory;
    namespace CarPlay = apmf::iface::com::stellantis::projectionIface::carplay;
    namespace AndroidAuto = apmf::iface::com::stellantis::projectionIface::androidAuto;

    using orpheus::service::ModeManagerService::Utility::readJsonFile;
    using orpheus::service::ModeManagerService::Utility::writeData;

    /**
     * @brief Construct a new Mode Manager Service object
     *
     */
    ModeManagerService::ModeManagerService()
    {
        LOG_F(INFO, "ModeManagerService: constructor");
        if (process == nullptr)
        {
            process = GetProcessObject();
        }

        if (!initAndCheckCarPlayModule())
        {
            LOG_F(WARNING, "CarPlay module initialization failed");
        }

        if (!initAndCheckAndroidAutoModule())
        {
            LOG_F(WARNING, "Android Auto module initialization failed");
        }
    }

    /**
     * @brief Destroy the Mode Manager Service object
     *
     */
    ModeManagerService::~ModeManagerService()
    {
        LOG_F(INFO, "ModeManagerService: object deleted");
        if (carPlayClient != nullptr || androidAutoClient != nullptr)
        {
            process = nullptr;
            carPlayClient = nullptr;
            androidAutoClient = nullptr;
        }
    }

    /**
     * @brief Returns a static instance of the class.
     *
     * @return ModeManagerService&
     */
    ModeManagerService &ModeManagerService::getInstance()
    {
        static ModeManagerService s_service;
        return s_service;
    }

    /**
     * @brief Called when client connects.
     *
     * @param[in] id
     */
    void ModeManagerService::onConnect(uint16_t id)
    {
        LOG_F(INFO, "Client {} connected.", id);
    }

    /**
     * @brief Called when client disconnects.
     *
     * @param[in] id
     */
    void ModeManagerService::onDisconnect(uint16_t id)
    {
        LOG_F(INFO, "Client {} disconnected.", id);
    }

    /**
     * @brief Get the Last Hmi Mode object
     *
     * @param[out] componentId
     * @param[out] path
     * @param[in] lastIndex
     * @return true
     * @return false
     */
    bool ModeManagerService::getLastHmiMode(string &componentId, string &path, int32_t lastIndex) const
    {
        string errorMsg;
        bool result{false};

        cJSON *const root{readJsonFile(FILEPATH, errorMsg)};
        if (root != nullptr)
        {
            int const index{cJSON_GetArraySize(root) - 1 - lastIndex};
            LOG_F(INFO, "{}, index: {}, {}", __FUNCTION__, index, lastIndex);
            if (index >= 0)
            {
                cJSON *const item{cJSON_GetArrayItem(root, index)};
                cJSON *const itemComponentId{cJSON_GetObjectItem(item, KEY_COMPONENT_ID.c_str())};
                componentId = string(itemComponentId->valuestring);
                path = string(cJSON_GetObjectItem(item, KEY_PATH.c_str())->valuestring);
                result = true;
            }
            LOG_F(INFO, "{}::componentId {}, path {}", __FUNCTION__, componentId, path);

            cJSON_Delete(root);
        }
        else
        {
            LOG_F(WARNING, "{} Get JSON data failed: {}", __FUNCTION__, errorMsg);
        }

        return result;
    }

    /**
     * @brief Get the Launch Param object
     *
     * @param[in] componentId
     * @return std::string
     */
    string ModeManagerService::getLaunchParam(string const &componentId)
    {
        // ToDo: Wait for the ModeManager design document and do the logic part implementation
        string launchParams{""};
        return launchParams;
    }

    /**
     * @brief Remove the component from JSON
     *
     * @param[in] root
     * @param[in] componentId
     */
    void ModeManagerService::removeComponentFromJson(cJSON *root, string const &componentId) const
    {
        int32_t const boundary{cJSON_GetArraySize(root) - 1};
        int32_t indexTarget{-1};
        for (int32_t i{boundary}; i >= 0; i--)
        {
            cJSON *const item{cJSON_GetArrayItem(root, i)};
            cJSON *const objectItem{cJSON_GetObjectItem(item, KEY_COMPONENT_ID.c_str())};
            if (objectItem != nullptr)
            {
                string const component{objectItem->valuestring};
                if (isSameComponent(component, componentId, false))
                {
                    indexTarget = i;
                    break;
                }
            }
        }
        if ((indexTarget >= 0) && (indexTarget <= boundary))
        {
            cJSON_DeleteItemFromArray(root, indexTarget);
        }
    }

    /**
     * @brief Remove the HMI mode
     *
     * @param[in] componentId
     * @param[out] errorMsg
     * @return true
     * @return false
     */
    bool ModeManagerService::removeHMIMode(string const &componentId, string &errorMsg) const
    {
        bool isSuccessful{false};

        cJSON *const root{readJsonFile(FILEPATH, errorMsg)};
        if (root != nullptr)
        {
            removeComponentFromJson(root, componentId);
            char *const jsonData{cJSON_PrintUnformatted(root)};
            string const jsonString{jsonData};

            if (writeData(FILEPATH, jsonString, errorMsg))
            {
                isSuccessful = true;
                LOG_F(INFO, "{} update Json content: {}", __FUNCTION__, jsonString);
            }
            else
            {
                LOG_F(ERROR, "{} update json content failed {}", __FUNCTION__, errorMsg);
            }

            cJSON_Delete(root);
            cJSON_free(jsonData);
        }
        else
        {
            LOG_F(WARNING, "{} Get JSON data failed: {}", __FUNCTION__, errorMsg);
        }

        return isSuccessful;
    }

    /**
     * @brief Update the HMI mode
     *
     * @param[in] componentId
     * @param[in] path
     * @param[out] errorMsg
     * @return true
     * @return false
     */
    bool ModeManagerService::updateHMIMode(string &componentId, string &path, string &errorMsg) const
    {
        bool isSuccessful{false};

        cJSON *root{readJsonFile(FILEPATH, errorMsg)};

        if (root == nullptr)
        {
            root = cJSON_CreateArray();
            LOG_F(INFO, "updateHMIMode Create new Json content");
        }
        else
        {
            removeComponentFromJson(root, componentId);
        }

        if (root != nullptr)
        {
            // ToDo: payload data implementation
            cJSON *const item{cJSON_CreateObject()};
            cJSON_AddItemToObject(item, KEY_COMPONENT_ID.c_str(), cJSON_CreateString(componentId.c_str()));
            cJSON_AddItemToObject(item, KEY_PATH.c_str(), cJSON_CreateString(path.c_str()));
            cJSON_AddItemToArray(root, item);
            LOG_F(INFO, "updateHMIMode Add Json content");

            char *const jsonData{cJSON_PrintUnformatted(root)};
            string const jsonString{jsonData};
            LOG_F(INFO, "updateHMIMode update Json content:: {}", jsonData);

            if (writeData(FILEPATH, jsonString, errorMsg))
            {
                isSuccessful = true;
            }
            else
            {
                errorMsg = "write data failed";
            }
            cJSON_Delete(root);
            cJSON_free(jsonData);
        }

        return isSuccessful;
    }

    /**
     * @brief Check whether the item is OneUnity item
     *
     * @param[in] item
     * @return true
     * @return false
     */
    bool ModeManagerService::isOneUnityItem(cJSON const *item) const
    {
        cJSON *const itemComponentOneUnity{cJSON_GetObjectItem(item, KEY_ONEUNITY.c_str())};
        bool result{false};
        if ((itemComponentOneUnity != nullptr) && static_cast<bool>(cJSON_IsBool(itemComponentOneUnity)))
        {
            result = static_cast<bool>(itemComponentOneUnity->valueint);
        }
        return result;
    }

    /**
     * @brief Check whether the component is Home
     *
     * @param[in] componentId
     * @return true
     * @return false
     */
    bool ModeManagerService::isHome(string componentId) const
    {
        return isSameComponent(componentId, HMI_MODE_DEFAULT_HOME, false);
    }

    /**
     * @brief Check whether the component is same
     *
     * @param[in] componentA
     * @param[in] componentB
     * @param[in] matchedParameters
     * @return true
     * @return false
     */
    bool ModeManagerService::isSameComponent(string componentA, string componentB, bool matchedParameters) const
    {
        bool isSame{false};
        if ((!componentA.empty()) && (!componentB.empty()))
        {
            string matchingTarget{componentA};
            string parameterTarget;
            std::size_t markIndex{matchingTarget.find('?')};
            if (markIndex != string::npos)
            {
                parameterTarget = matchingTarget.substr(markIndex + 1U);
                matchingTarget = matchingTarget.substr(0U, markIndex);
            }
            string matchingPair{componentB};
            string parameterPair;
            markIndex = matchingPair.find('?');
            if (markIndex != string::npos)
            {
                parameterPair = matchingPair.substr(markIndex + 1U);
                matchingPair = matchingPair.substr(0U, markIndex);
            }
            if (matchingPair == matchingTarget)
            {
                if (matchedParameters)
                {
                    isSame = parameterPair == parameterTarget;
                }
                else
                {
                    isSame = true;
                }
            }
        }

        return isSame;
    }

    /**
     * @brief Notify the state event subscribers
     *
     * @param[in] state
     * @param[in] errorMsg
     */
    void ModeManagerService::notifyStateSubscribers(StateOperation &state, string &errorMsg)
    {
        LOG_F(INFO, "{}", __FUNCTION__);
        for (auto &[subscriptionId, subscription] : m_clientStateSubscriptions)
        {
            auto const callback = subscription.getCallback();
            static_cast<void>(callback(state, errorMsg, subscription.getContext()));
        }
    }

    /**
     * @brief Notify the back stack event subscribers
     *
     * @param[in] componentId
     * @param[in] path
     * @param[in] errorMsg
     */
    void ModeManagerService::notifyBackStackSubscribers(string &componentId, string &path, string &errorMsg)
    {
        LOG_F(INFO, "{}", __FUNCTION__);
        for (auto &[subscriptionId, subscription] : m_clientBackStackSubscriptions)
        {
            auto const callback = subscription.getCallback();
            static_cast<void>(callback(componentId, path, errorMsg, subscription.getContext()));
        }
    }

    /**
     * @brief Process the Transient mode
     *
     * @param[in] componentId
     * @return int32_t 0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::processTransientMode(string const &componentId)
    {
        // ToDo: Wait for the design document and to do the logic part implementation
        if (componentId.empty())
        {
            // ToDo: remove TransientMode
            transientMode = "";
        }
        else
        {
            transientMode = componentId;
        }
        LOG_F(INFO, "{}, update transientMode:{}", __FUNCTION__, transientMode);
        return 0;
    }

    /**
     * @brief Get the current mode
     *
     * @return std::string
     */
    string ModeManagerService::retrieveCurrentMode() const
    {
        string hmiMode;
        string hmiPath;

        if (!getLastHmiMode(hmiMode, hmiPath, 0))
        {
            LOG_F(ERROR, "Unable to retrieve last HMI mode");
        }

        return hmiMode;
    }

    /**
     * @brief Initialize the CarPlay module
     *
     * @return true
     * @return false
     */
    bool ModeManagerService::initAndCheckCarPlayModule()
    {
        if (carPlayClient == nullptr)
        {
            try
            {
                auto const carPlayClientFactory = process->getComponent("/com.stellantis.projectionIface.carplay").TryQueryInterface<ICarPlayClientFactory>();
                carPlayClient = carPlayClientFactory->makeCarPlayClient();
            }
            catch (BaseError const &ex)
            {
                LOG_F(WARNING, "carPlayClient init fail = {}", ex.what());
            }
        }

        return (carPlayClient != nullptr);
    }

    /**
     * @brief Inital the Android Auto module
     *
     * @return true
     * @return false
     */
    bool ModeManagerService::initAndCheckAndroidAutoModule()
    {
        if (androidAutoClient == nullptr)
        {
            try
            {
                auto const androidAutoClientFactory = process->getComponent("/com.stellantis.projectionIface.androidAuto").TryQueryInterface<IAndroidAutoClientFactory>();
                androidAutoClient = androidAutoClientFactory->makeAndroidAutoClient();
            }
            catch (BaseError const &ex)
            {
                LOG_F(WARNING, "androidAutoClient init fail = {}", ex.what());
            }
        }

        return (androidAutoClient != nullptr);
    }

    /**
     * @brief Notify CarPlay and Android Auto screen transition during mode update
     *
     */
    void ModeManagerService::handleProjectionDuringModeUpdate()
    {
        string const currentComponentID{retrieveCurrentMode()};
        CarPlay::ERROR_ID_E carPlayErrMsg;
        AndroidAuto::ERROR_ID_E androidAutoErrMsg;

        if (currentComponentID == PROJECTION_DEMO)
        {
            carPlayErrMsg = notifyCarPlayScreenTransitionType(CarPlay::SCREEN_TYPE_E::SCREEN_TYPE_TAKE_SCREEN, true);
            androidAutoErrMsg = notifyAndroidAutoScreenTransitionType(AndroidAuto::SCREEN_TYPE_E::SCREEN_TYPE_TAKE_SCREEN);
        }
        else
        {
            carPlayErrMsg = notifyCarPlayUserScreenTransition(CarPlay::LASTMODE_E::LASTMODE_NATIVE);
            androidAutoErrMsg = notifyAndroidAutoUserScreenTransition(AndroidAuto::LASTMODE_E::LASTMODE_NATIVE);
        }

        if (carPlayErrMsg == CarPlay::ERROR_ID_E::NOT_CONNECTED)
        {
            LOG_F(WARNING, "carPlayErrMsg:: NOT_CONNECTED");
        }
        if (androidAutoErrMsg == AndroidAuto::ERROR_ID_E::NOT_CONNECTED)
        {
            LOG_F(WARNING, "androidAutoErrMsg:: NOT_CONNECTED");
        }
    }

    /**
     * @brief API to update current mode with the given component.
     *
     * @param[in] componentId   App ID or ComponentID (OneUnity component)
     * @param[in] path          Path data for an OneUnity component
     * @return int32_t  0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::requestModeUpdate(string &componentId, string &path)
    {
        LOG_F(INFO, "{}", __FUNCTION__);
        int32_t result{0};

        string errorMsg;

        if (!path.empty())
        {
            // ToDo: path data implementation;
        }

        // The CAR_PLAY_ID will replace PROJECTION_DEMO
        if (componentId != PROJECTION_DEMO)
        {
            handleProjectionDuringModeUpdate();
        }
        if (!updateHMIMode(componentId, path, errorMsg))
        {
            LOG_F(ERROR, "updateHMIMode failed: {}", errorMsg);
            result = -1;
        }

        return result;
    }

    /**
     * @brief API to set request transient mode update.
     *
     * @param[in] componentId   App ID or ComponentID (OneUnity component)
     * @return int32_t  0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::requestTransientModeUpdate(string &componentId)
    {
        LOG_F(INFO, "{}", __FUNCTION__);

        // ToDo: Wait for the ModeManager design document and do the logic part implementation
        int32_t result{0};
        string const currentComponentID{retrieveCurrentMode()};

        // The CAR_PLAY_ID will replace PROJECTION_DEMO
        if (currentComponentID == PROJECTION_DEMO)
        {
            handleCarPlayNotification(componentId);
            handleAndroidAutoNotification(componentId);
        }
        if (processTransientMode(componentId) != 0)
        {
            LOG_F(ERROR, "processTransientMode failed");
            result = -1;
        }

        return result;
    }

    /**
     * @brief API to switch back to the last mode
     *
     * @return int32_t  0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::popBackStack()
    {
        string errorMsg;
        string hmiMode;
        string hmiPath;
        const bool hasLastHmiMode{getLastHmiMode(hmiMode, hmiPath, 0)};
        LOG_F(INFO, "popBackStack {} hasLastHmiMode {}", hmiMode, hasLastHmiMode);

        if (hasLastHmiMode)
        {
            if (removeHMIMode(hmiMode, errorMsg))
            {
                notifyBackStackSubscribers(hmiMode, hmiPath, errorMsg);
            }
            else
            {
                LOG_F(WARNING, "popBackStack removeHMIMode failed: {}", errorMsg);
            }
        }
        else
        {
            string componentId{HMI_MODE_FOREGROUND_ID};
            string path{""};
            errorMsg = "No last mode";
            notifyBackStackSubscribers(componentId, path, errorMsg);
            LOG_F(INFO, "popBackStack No Last Mode!");
        }

        return 0;
    }

    /**
     * @brief API to set pending request with the given component.
     *
     * @param[in] componentId   App ID or ComponentID (OneUnity component)
     * @return int32_t  0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::setPendingRequest(string &componentId)
    {
        LOG_F(INFO, "{}", __FUNCTION__);

        string errorMsg;
        StateOperation state{StateOperation::ALLOWED};
        // ToDo: Wait for the ModeManager design document and do the logic part implementation

        notifyStateSubscribers(state, errorMsg);
        return 0;
    }

    /**
     * @brief API to request launch params with the given component.
     *
     * @param[in] componentId   App ID or ComponentID (OneUnity component)
     * @param[in] requestUuid   Universally Unique Identifier
     * @param[in] callback      Callback to be invoked with the result of the addition
     * @param[in] result_ctx    Context that is passed back with the result callback
     * @return int32_t  0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::iRequestLaunchParams(string &componentId, string &requestUuid, onLaunchParamCallback callback, void *result_ctx)
    {
        LOG_F(INFO, "{}", __FUNCTION__);

        string errorMsg;
        string launchParams{getLaunchParam(componentId)};
        // ToDo: Wait for the ModeManager design document and do the logic part implementation

        static_cast<void>(callback(componentId, launchParams, requestUuid, errorMsg, result_ctx));
        return 0;
    }

    /**
     * @brief API to subscribe state event
     *
     * @param[in] callback  Callback to be invoked with the result of the addition
     * @param[in] ctx       Context that is passed back with the result callback
     * @return int32_t  0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::stateSubscribe(onStateCallback callback, void *ctx)
    {
        LOG_F(INFO, "{}", __FUNCTION__);

        clientInfo serviceClientInfo{};
        ModeManagerServiceSkeleton::getInstance().getClientInfo(&serviceClientInfo);

        StateSubscriberData const subscriber{callback, ctx};

        static_cast<void>(m_clientStateSubscriptions.emplace(serviceClientInfo.client_id, subscriber));

        return 0;
    }

    /**
     * @brief API to unsubscribe state event
     *
     * @return int32_t  0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::stateUnsubscribe()
    {
        LOG_F(INFO, "{}", __FUNCTION__);
        int32_t result{0};

        clientInfo serviceClientInfo{};
        ModeManagerServiceSkeleton::getInstance().getClientInfo(&serviceClientInfo);

        auto const it = m_clientStateSubscriptions.find(serviceClientInfo.client_id);
        if (it != m_clientStateSubscriptions.end())
        {
            static_cast<void>(m_clientStateSubscriptions.erase(it));
        }
        else
        {
            result = -1;
        }
        return result;
    }

    /**
     * @brief API to subscribe back stack event
     *
     * @param[in] callback  Callback to be invoked with the result of the addition
     * @param[in] ctx       Context that is passed back with the result callback
     * @return int32_t  0 for success. Any other value is an error.
     */
    int32_t ModeManagerService::backStackSubscribe(onBackStackCallback callback, void *ctx)
    {
        LOG_F(INFO, "{}", __FUNCTION__);

        clientInfo serviceClientInfo{};
        ModeManagerServiceSkeleton::getInstance().getClientInfo(&serviceClientInfo);

        BackStackSubscriberData const subscriber{callback, ctx};

        static_cast<void>(m_clientBackStackSubscriptions.emplace(serviceClientInfo.client_id, subscriber));

        return 0;
    }

    /**
     * @brief  API to unsubscribe back stack event
     *
     * @return int32_t
     */
    int32_t ModeManagerService::backStackUnsubscribe()
    {
        LOG_F(INFO, "{}", __FUNCTION__);
        int32_t result{0};

        clientInfo serviceClientInfo{};
        ModeManagerServiceSkeleton::getInstance().getClientInfo(&serviceClientInfo);

        auto const it = m_clientBackStackSubscriptions.find(serviceClientInfo.client_id);
        if (it != m_clientBackStackSubscriptions.end())
        {
            static_cast<void>(m_clientBackStackSubscriptions.erase(it));
        }
        else
        {
            result = -1;
        }
        return result;
    }

    /**
     * @brief  Notify the CarPlay screen transition type
     *
     * @param[in] screenType
     * @param[in] status
     * @return CarPlay::ERROR_ID_E
     */
    CarPlay::ERROR_ID_E ModeManagerService::notifyCarPlayScreenTransitionType(CarPlay::SCREEN_TYPE_E screenType, bool status)
    {
        CarPlay::ERROR_ID_E error{CarPlay::ERROR_ID_E::NO_ERR};
        if (initAndCheckCarPlayModule())
        {
            error = carPlayClient->notifyScreenTransitionType(screenType, status);
        }
        return error;
    }

    /**
     * @brief Notify the Android Auto screen transition type
     *
     * @param[in] screenType
     * @return AndroidAuto::ERROR_ID_E
     */
    AndroidAuto::ERROR_ID_E ModeManagerService::notifyAndroidAutoScreenTransitionType(AndroidAuto::SCREEN_TYPE_E screenType)
    {
        AndroidAuto::ERROR_ID_E error{AndroidAuto::ERROR_ID_E::NO_ERR};
        if (initAndCheckAndroidAutoModule())
        {
            error = androidAutoClient->notifyScreenTransitionType(screenType);
        }
        return error;
    }

    /**
     * @brief Notify the CarPlay user screen transition
     *
     * @param[in] lastMode
     * @return CarPlay::ERROR_ID_E
     */
    CarPlay::ERROR_ID_E ModeManagerService::notifyCarPlayUserScreenTransition(CarPlay::LASTMODE_E lastMode)
    {
        CarPlay::ERROR_ID_E error{CarPlay::ERROR_ID_E::NO_ERR};
        if (initAndCheckCarPlayModule())
        {
            error = carPlayClient->notifyUserScreenTransition(lastMode);
        }
        return error;
    }

    /**
     * @brief Notify the Android Auto user screen transition
     *
     * @param[in] lastMode
     * @return AndroidAuto::ERROR_ID_E
     */
    AndroidAuto::ERROR_ID_E ModeManagerService::notifyAndroidAutoUserScreenTransition(AndroidAuto::LASTMODE_E lastMode)
    {
        AndroidAuto::ERROR_ID_E error{AndroidAuto::ERROR_ID_E::NO_ERR};
        if (initAndCheckAndroidAutoModule())
        {
            error = androidAutoClient->notifyUserScreenTransition(lastMode);
        }
        return error;
    }

    /**
     * @brief Notify CarPlay screen transition
     *
     * @param[in] componentId
     */
    void ModeManagerService::handleCarPlayNotification(std::string const &componentId)
    {
        CarPlay::ERROR_ID_E carPlayErrMsg;
        if (componentId == APL_CARD_ID)
        {
            carPlayErrMsg = notifyCarPlayScreenTransitionType(CarPlay::SCREEN_TYPE_E::SCREEN_TYPE_SPEECH, true);
        }
        else if (componentId == POPUP_COMPONENT_ID)
        {
            carPlayErrMsg = notifyCarPlayScreenTransitionType(CarPlay::SCREEN_TYPE_E::SCREEN_TYPE_NOTIFICATION_POPUP, true);
        }
        else if (componentId.empty())
        {
            if (transientMode == APL_CARD_ID)
            {
                carPlayErrMsg = notifyCarPlayScreenTransitionType(CarPlay::SCREEN_TYPE_E::SCREEN_TYPE_SPEECH, false);
            }
            else if (transientMode == POPUP_COMPONENT_ID)
            {
                carPlayErrMsg = notifyCarPlayScreenTransitionType(CarPlay::SCREEN_TYPE_E::SCREEN_TYPE_NOTIFICATION_POPUP, false);
            }
            else
            {
                // No action needed, but else block is required for static analysis
            }
        }
        else
        {
            // No action needed, but else block is required for static analysis
        }
        if (carPlayErrMsg == CarPlay::ERROR_ID_E::NOT_CONNECTED)
        {
            LOG_F(WARNING, "carPlayErrMsg:: NOT_CONNECTED");
        }
    }

    /**
     * @brief Notify AndroidAuto screen transition
     *
     * @param[in] componentId
     */
    void ModeManagerService::handleAndroidAutoNotification(std::string const &componentId)
    {
        AndroidAuto::ERROR_ID_E androidAutoErrMsg;
        if (componentId == APL_CARD_ID)
        {
            androidAutoErrMsg = notifyAndroidAutoScreenTransitionType(AndroidAuto::SCREEN_TYPE_E::SCREEN_TYPE_SPEECH);
        }
        else if (componentId == POPUP_COMPONENT_ID)
        {
            androidAutoErrMsg = notifyAndroidAutoScreenTransitionType(AndroidAuto::SCREEN_TYPE_E::SCREEN_TYPE_NOTIFICATION_POPUP);
        }
        else if (componentId.empty())
        {
            androidAutoErrMsg = notifyAndroidAutoScreenTransitionType(AndroidAuto::SCREEN_TYPE_E::SCREEN_TYPE_NONE);
        }
        else
        {
            // No action needed, but else block is required for static analysis
        }
        if (androidAutoErrMsg == AndroidAuto::ERROR_ID_E::NOT_CONNECTED)
        {
            LOG_F(WARNING, "androidAutoErrMsg:: NOT_CONNECTED");
        }
    }
} // namespace orpheus::service::ModeManagerService
