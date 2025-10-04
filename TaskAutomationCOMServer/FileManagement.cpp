#include "pch.h"
#include "FileManagement.h"
#include "StringUtils.h"
#include "FileUtils.h"
#include "Logger.h"
#include <filesystem>
#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <atlstr.h>

namespace fs = std::filesystem;

STDMETHODIMP CFileManagement::BatchRenameFiles(BSTR directoryPath, BSTR replaceChar, VARIANT_BOOL includeSubdirectories, BSTR fileTypeFilter, BSTR* result)
{
    std::vector<std::string> messages;
    int filesProcessed = 0;
    int filesRenamed = 0;
    int errors = 0;

    try
    {
        // Validate result parameter
        if (!result)
        {
            Logger::Error("BatchRenameFiles: result parameter is null");
            return E_POINTER;
        }

        *result = nullptr;

        // Validate input parameters
        if (StringUtils::IsNullOrEmpty(directoryPath))
        {
            *result = StringUtils::StringToBSTR("Error: Directory path cannot be empty");
            Logger::Error("BatchRenameFiles: Directory path is empty");
            return E_INVALIDARG;
        }

        if (StringUtils::IsNullOrEmpty(replaceChar))
        {
            *result = StringUtils::StringToBSTR("Error: Replace character cannot be empty");
            Logger::Error("BatchRenameFiles: Replace character is empty");
            return E_INVALIDARG;
        }

        if (StringUtils::IsNullOrEmpty(fileTypeFilter))
        {
            *result = StringUtils::StringToBSTR("Error: File type filter cannot be empty");
            Logger::Error("BatchRenameFiles: File type filter is empty");
            return E_INVALIDARG;
        }

        // Convert parameters
        std::string dirPath = StringUtils::BSTRToString(directoryPath);
        std::string replace = StringUtils::BSTRToString(replaceChar);
        std::string filter = StringUtils::BSTRToString(fileTypeFilter);

        // Validate directory
        std::string errorMsg;
        if (!FileUtils::ValidateDirectory(dirPath, errorMsg))
        {
            *result = StringUtils::StringToBSTR("Error: " + errorMsg);
            Logger::Error("BatchRenameFiles: " + errorMsg);
            return E_INVALIDARG;
        }

        // Validate regex pattern
        if (!FileUtils::ValidateRegexPattern(filter, errorMsg))
        {
            *result = StringUtils::StringToBSTR("Error: " + errorMsg);
            Logger::Error("BatchRenameFiles: " + errorMsg);
            return E_INVALIDARG;
        }

        std::regex fileRegex(filter);
        char replaceCharValue = replace.empty() ? '_' : replace[0];

        Logger::Info("BatchRenameFiles: Starting operation on directory: " + dirPath);

        // Use appropriate iterator based on includeSubdirectories flag
        if (includeSubdirectories)
        {
            // Recursive iteration
            for (const auto& entry : fs::recursive_directory_iterator(dirPath))
            {
                if (!fs::is_regular_file(entry.path()))
                    continue;

                filesProcessed++;
                std::string filename = entry.path().filename().string();

                if (std::regex_match(filename, fileRegex))
                {
                    std::string newFilename = filename;
                    std::replace(newFilename.begin(), newFilename.end(), ' ', replaceCharValue);

                    if (newFilename != filename)
                    {
                        fs::path newPath = entry.path().parent_path() / newFilename;

                        if (FileUtils::SafeRename(entry.path(), newPath, errorMsg))
                        {
                            filesRenamed++;
                            Logger::Info("Renamed: " + filename + " -> " + newFilename);
                        }
                        else
                        {
                            errors++;
                            messages.push_back("Failed to rename '" + filename + "': " + errorMsg);
                            Logger::Warning("Failed to rename '" + filename + "': " + errorMsg);
                        }
                    }
                }
            }
        }
        else
        {
            // Non-recursive iteration
            for (const auto& entry : fs::directory_iterator(dirPath))
            {
                if (!fs::is_regular_file(entry.path()))
                    continue;

                filesProcessed++;
                std::string filename = entry.path().filename().string();

                if (std::regex_match(filename, fileRegex))
                {
                    std::string newFilename = filename;
                    std::replace(newFilename.begin(), newFilename.end(), ' ', replaceCharValue);

                    if (newFilename != filename)
                    {
                        fs::path newPath = entry.path().parent_path() / newFilename;

                        if (FileUtils::SafeRename(entry.path(), newPath, errorMsg))
                        {
                            filesRenamed++;
                            Logger::Info("Renamed: " + filename + " -> " + newFilename);
                        }
                        else
                        {
                            errors++;
                            messages.push_back("Failed to rename '" + filename + "': " + errorMsg);
                            Logger::Warning("Failed to rename '" + filename + "': " + errorMsg);
                        }
                    }
                }
            }
        }

        // Build result message
        std::vector<std::string> resultLines;
        resultLines.push_back("Batch Rename Operation Completed");
        resultLines.push_back("Files processed: " + std::to_string(filesProcessed));
        resultLines.push_back("Files renamed: " + std::to_string(filesRenamed));
        resultLines.push_back("Errors: " + std::to_string(errors));

        if (!messages.empty())
        {
            resultLines.push_back("");
            resultLines.push_back("Error Details:");
            for (const auto& msg : messages)
            {
                resultLines.push_back("  " + msg);
            }
        }

        std::string resultMessage = StringUtils::BuildResultMessage(resultLines);
        *result = StringUtils::StringToBSTR(resultMessage);

        Logger::Info("BatchRenameFiles: Completed - " + std::to_string(filesRenamed) + " files renamed, " + std::to_string(errors) + " errors");

        return S_OK;
    }
    catch (const fs::filesystem_error& e)
    {
        std::string errorMessage = "Filesystem error: " + std::string(e.what());
        *result = StringUtils::StringToBSTR(errorMessage);
        Logger::Error("BatchRenameFiles: " + errorMessage);
        return E_FAIL;
    }
    catch (const std::exception& e)
    {
        std::string errorMessage = "Unexpected error: " + std::string(e.what());
        *result = StringUtils::StringToBSTR(errorMessage);
        Logger::Error("BatchRenameFiles: " + errorMessage);
        return E_FAIL;
    }
}

STDMETHODIMP CFileManagement::OrganizeDirectory(BSTR directoryPath, BSTR criteria, BSTR* result)
{
    std::vector<std::string> messages;
    int filesProcessed = 0;
    int filesOrganized = 0;
    int errors = 0;

    try
    {
        // Validate result parameter
        if (!result)
        {
            Logger::Error("OrganizeDirectory: result parameter is null");
            return E_POINTER;
        }

        *result = nullptr;

        // Validate input parameters
        if (StringUtils::IsNullOrEmpty(directoryPath))
        {
            *result = StringUtils::StringToBSTR("Error: Directory path cannot be empty");
            Logger::Error("OrganizeDirectory: Directory path is empty");
            return E_INVALIDARG;
        }

        if (StringUtils::IsNullOrEmpty(criteria))
        {
            *result = StringUtils::StringToBSTR("Error: Criteria cannot be empty");
            Logger::Error("OrganizeDirectory: Criteria is empty");
            return E_INVALIDARG;
        }

        // Convert parameters
        std::string dirPath = StringUtils::BSTRToString(directoryPath);
        std::string crit = StringUtils::BSTRToString(criteria);

        // Validate directory
        std::string errorMsg;
        if (!FileUtils::ValidateDirectory(dirPath, errorMsg))
        {
            *result = StringUtils::StringToBSTR("Error: " + errorMsg);
            Logger::Error("OrganizeDirectory: " + errorMsg);
            return E_INVALIDARG;
        }

        Logger::Info("OrganizeDirectory: Starting operation on directory: " + dirPath + " with criteria: " + crit);

        // Iterate through files in the directory (non-recursive)
        for (const auto& entry : fs::directory_iterator(dirPath))
        {
            if (!fs::is_regular_file(entry.path()))
                continue;

            filesProcessed++;

            // Determine target folder based on criteria
            std::string targetFolderName = FileUtils::DetermineOrganizationFolder(entry.path(), crit);
            fs::path targetDir = fs::path(dirPath) / targetFolderName;

            // Create target directory
            if (!FileUtils::SafeCreateDirectories(targetDir, errorMsg))
            {
                errors++;
                messages.push_back("Failed to create directory '" + targetFolderName + "': " + errorMsg);
                Logger::Warning("Failed to create directory '" + targetFolderName + "': " + errorMsg);
                continue;
            }

            // Move file to target directory
            fs::path newPath = targetDir / entry.path().filename();

            if (FileUtils::SafeRename(entry.path(), newPath, errorMsg))
            {
                filesOrganized++;
                Logger::Info("Moved: " + entry.path().filename().string() + " -> " + targetFolderName + "/");
            }
            else
            {
                errors++;
                messages.push_back("Failed to move '" + entry.path().filename().string() + "': " + errorMsg);
                Logger::Warning("Failed to move '" + entry.path().filename().string() + "': " + errorMsg);
            }
        }

        // Build result message
        std::vector<std::string> resultLines;
        resultLines.push_back("Directory Organization Completed");
        resultLines.push_back("Files processed: " + std::to_string(filesProcessed));
        resultLines.push_back("Files organized: " + std::to_string(filesOrganized));
        resultLines.push_back("Errors: " + std::to_string(errors));
        resultLines.push_back("Organization criteria: " + crit);

        if (!messages.empty())
        {
            resultLines.push_back("");
            resultLines.push_back("Error Details:");
            for (const auto& msg : messages)
            {
                resultLines.push_back("  " + msg);
            }
        }

        std::string resultMessage = StringUtils::BuildResultMessage(resultLines);
        *result = StringUtils::StringToBSTR(resultMessage);

        Logger::Info("OrganizeDirectory: Completed - " + std::to_string(filesOrganized) + " files organized, " + std::to_string(errors) + " errors");

        return S_OK;
    }
    catch (const fs::filesystem_error& e)
    {
        std::string errorMessage = "Filesystem error: " + std::string(e.what());
        *result = StringUtils::StringToBSTR(errorMessage);
        Logger::Error("OrganizeDirectory: " + errorMessage);
        return E_FAIL;
    }
    catch (const std::exception& e)
    {
        std::string errorMessage = "Unexpected error: " + std::string(e.what());
        *result = StringUtils::StringToBSTR(errorMessage);
        Logger::Error("OrganizeDirectory: " + errorMessage);
        return E_FAIL;
    }
}
