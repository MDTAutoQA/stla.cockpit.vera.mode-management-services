#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <filesystem>
#include <cJSON/cJSON.h>

namespace orpheus::service::ModeManagerService::Utility
{
    /**
     * @brief Check whether the directory exists
     *
     * @param directory
     * @return true
     * @return false
     */
    bool hasDirectory(const std::filesystem::path &directory);

    /**
     * @brief Write the string to text files
     *
     * @param[in] filePath
     * @param[in] value
     * @param[out] errorMsg
     * @return true
     * @return false
     */
    bool writeData(std::string const &filePath, std::string const &value, std::string &errorMsg);

    /**
     * @brief Read text file
     *
     * @param[in] filePath
     * @param[out] value
     * @param[out] errorMsg
     * @return true
     * @return false
     */
    bool readData(std::string const &filePath, std::string &value, std::string &errorMsg);

    /**
     * @brief Read json file
     *
     * @param[in] filePath
     * @param[out] errorMsg
     * @return cJSON*
     */
    cJSON *readJsonFile(std::string const &filePath, std::string &errorMsg);
} // namespace orpheus::service::ModeManagerService::Utility

#endif // UTILITY_H