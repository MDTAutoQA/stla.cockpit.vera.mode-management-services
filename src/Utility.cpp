#include "Utility.h"
#include <fstream>

namespace orpheus::service::ModeManagerService::Utility
{
    using std::string;

    /**
     * @brief Check whether the directory exists
     *
     * @param directory
     * @return true
     * @return false
     */
    bool hasDirectory(const std::filesystem::path &directory)
    {
        bool success{true};
        if (!std::filesystem::exists(directory))
        {
            bool const isDirCreated{std::filesystem::create_directories(directory)};
            if (!isDirCreated)
            {
                success = false;
            }
        }
        return success;
    }

    /**
     * @brief Write the string to text files
     *
     * @param[in] filePath
     * @param[in] value
     * @param[out] errorMsg
     * @return true
     * @return false
     */
    bool writeData(string const &filePath, string const &value, string &errorMsg)
    {
        bool isWriteSuccess{false};
        std::filesystem::path const directory{std::filesystem::path(filePath).parent_path()};
        if (hasDirectory(directory))
        {
            std::ofstream outFile{filePath, std::ofstream::binary};
            outFile << value;
            outFile.close();
            isWriteSuccess = true;
        }
        else
        {
            errorMsg = "Create directory failed";
        }
        return isWriteSuccess;
    }

    /**
     * @brief Read text file
     *
     * @param[in] filePath
     * @param[out] value
     * @param[out] errorMsg
     * @return true
     * @return false
     */
    bool readData(string const &filePath, string &value, string &errorMsg)
    {
        bool isReadSuccess{false};
        if (std::filesystem::exists(filePath))
        {
            std::ifstream inFile{filePath, std::ifstream::binary};
            if (inFile.is_open())
            {
                std::stringstream buffer;
                buffer << inFile.rdbuf();
                value = buffer.str();
                inFile.close();
                isReadSuccess = true;
            }
            else
            {
                errorMsg = "Open file failed";
            }
        }
        else
        {
            errorMsg = "File not found";
            isReadSuccess = true;
        }
        return isReadSuccess;
    }

    /**
     * @brief Read json file
     *
     * @param[in] filePath
     * @param[out] errorMsg
     * @return cJSON*
     */
    cJSON *readJsonFile(string const &filePath, string &errorMsg)
    {
        string dataStr;
        cJSON *root{nullptr};
        if (readData(filePath, dataStr, errorMsg) && (!dataStr.empty()))
        {
            root = cJSON_Parse(dataStr.c_str());
            if (root == nullptr)
            {
                errorMsg = "Get JSON parse failed";
            }
        }
        else
        {
            errorMsg = errorMsg.empty() ? "Read file failed or file is empty" : errorMsg;
        }
        return root;
    }
} // namespace orpheus::service::ModeManagerService::Utility
