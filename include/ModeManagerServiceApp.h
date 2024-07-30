#ifndef ModeManager_SERVICE_APP_H_
#define ModeManager_SERVICE_APP_H_

// Include the LCM headers
#include <orpheus/framework/app/Application.h>
#include <orpheus/framework/app/ApplicationContext.h>
#include <orpheus/framework/lifecycle-manager/LcmClient.h>

namespace orpheus::service::ModeManagerService
{
    /**
     * @brief Service Application class
     * 
     */
    class ModeManagerServiceApp : public orpheus::framework::app::ServiceApplication
    {
    public:
        using orpheus::framework::app::ServiceApplication::ServiceApplication;

        /**
         * @brief Delete the copy constructor
         * 
         */
        ModeManagerServiceApp(const ModeManagerServiceApp &) = delete;

        /**
         * @brief Delete the move constructor
         * 
         */
        ModeManagerServiceApp(ModeManagerServiceApp &&) = delete;

        /**
         * @brief Delete the copy assignment operator
         * 
         * @return ModeManagerServiceApp& 
         */
        ModeManagerServiceApp &operator=(const ModeManagerServiceApp &) = delete;

        /**
         * @brief Delete the move assignment operator
         * 
         * @return ModeManagerServiceApp& 
         */
        ModeManagerServiceApp &operator=(ModeManagerServiceApp &&) = delete;

        /**
         * @brief Destroy the Mode Manager Service App object
         * 
         */
        ~ModeManagerServiceApp() override = default;

        /**
         * @brief Lifecycle method called when service is being started
         *
         * @return int
         */
        int onStart() override;

        /**
         * @brief Lifecycle method called when service is being stopped
         *
         * @return int
         */
        int onStop() override;
    };

} // namespace orpheus::sample::ModeManagerService

#endif // ModeManager_SERVICE_APP_H_