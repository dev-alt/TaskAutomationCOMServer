//=============================================================================
// FileManagement.cpp
//=============================================================================
//
// PURPOSE:
//   Implementation of the CFileManagement COM class.
//   Provides file management automation operations exposed through COM interfaces.
//
// KEY CONCEPTS:
//   - Uses C++17 std::filesystem for cross-platform file operations
//   - BSTR (Basic String): COM's native string type (wide char, length-prefixed)
//   - HRESULT: Standard COM return type (32-bit value indicating success/failure)
//   - Exception safety: All C++ exceptions are caught and converted to HRESULTs
//
//=============================================================================

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

// Namespace alias for convenience - std::filesystem paths can be verbose
namespace fs = std::filesystem;

//=============================================================================
// CFileManagement::BatchRenameFiles
//=============================================================================
// Batch renames files in a directory by replacing spaces with a specified character.
//
// ALGORITHM:
//   1. Validate all input parameters (directory exists, regex is valid, etc.)
//   2. Convert COM BSTR strings to C++ std::string for easier manipulation
//   3. Iterate through directory (recursive or non-recursive based on flag)
//   4. For each file matching the regex pattern:
//      a. Replace all spaces with the specified replacement character
//      b. Attempt to rename the file using std::filesystem::rename
//      c. Log success/failure and collect statistics
//   5. Build a detailed result message with statistics and any error details
//   6. Return S_OK (operation completed) or error HRESULT
//
// ERROR HANDLING:
//   - Parameter validation errors return E_INVALIDARG
//   - Null pointer for result returns E_POINTER
//   - Individual file rename failures are logged but don't stop the operation
//   - Filesystem exceptions are caught and returned as E_FAIL
//
// MEMORY MANAGEMENT:
//   - The returned BSTR must be freed by the caller using SysFreeString()
//   - ATL's CComBSTR is used internally for automatic BSTR management
//
//=============================================================================
STDMETHODIMP CFileManagement::BatchRenameFiles(BSTR directoryPath, BSTR replaceChar, VARIANT_BOOL includeSubdirectories, BSTR fileTypeFilter, BSTR* result)
{
    // Statistics tracking for operation summary
    std::vector<std::string> messages;  // Collect error messages for detailed reporting
    int filesProcessed = 0;              // Total files examined
    int filesRenamed = 0;                // Successfully renamed files
    int errors = 0;                      // Count of failures

    try
    {
        //---------------------------------------------------------------------
        // STEP 1: Parameter Validation
        //---------------------------------------------------------------------
        // COM best practice: Always validate [out] parameters first
        // If result is NULL, we can't return error information to the caller
        if (!result)
        {
            Logger::Error("BatchRenameFiles: result parameter is null");
            return E_POINTER;  // Standard COM error for NULL pointer
        }

        // Initialize output parameter to NULL (COM best practice)
        *result = nullptr;

        // Validate required input parameters using helper function
        // BSTR can be NULL or empty, so we need to check both cases
        if (StringUtils::IsNullOrEmpty(directoryPath))
        {
            *result = StringUtils::StringToBSTR("Error: Directory path cannot be empty");
            Logger::Error("BatchRenameFiles: Directory path is empty");
            return E_INVALIDARG;  // Standard COM error for invalid argument
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

        //---------------------------------------------------------------------
        // STEP 2: Convert COM types to C++ types
        //---------------------------------------------------------------------
        // BSTR is a Windows-specific string type (wide char, length-prefixed)
        // Convert to std::string for easier manipulation with C++ standard library
        std::string dirPath = StringUtils::BSTRToString(directoryPath);
        std::string replace = StringUtils::BSTRToString(replaceChar);
        std::string filter = StringUtils::BSTRToString(fileTypeFilter);

        //---------------------------------------------------------------------
        // STEP 3: Validate directory exists and is accessible
        //---------------------------------------------------------------------
        std::string errorMsg;
        if (!FileUtils::ValidateDirectory(dirPath, errorMsg))
        {
            *result = StringUtils::StringToBSTR("Error: " + errorMsg);
            Logger::Error("BatchRenameFiles: " + errorMsg);
            return E_INVALIDARG;
        }

        //---------------------------------------------------------------------
        // STEP 4: Validate regex pattern
        //---------------------------------------------------------------------
        // If the regex is invalid, std::regex constructor throws an exception
        // We validate it first to provide a better error message to the user
        if (!FileUtils::ValidateRegexPattern(filter, errorMsg))
        {
            *result = StringUtils::StringToBSTR("Error: " + errorMsg);
            Logger::Error("BatchRenameFiles: " + errorMsg);
            return E_INVALIDARG;
        }

        // Compile the regex pattern for file matching
        std::regex fileRegex(filter);

        // Extract the first character for replacement (default to underscore)
        char replaceCharValue = replace.empty() ? '_' : replace[0];

        Logger::Info("BatchRenameFiles: Starting operation on directory: " + dirPath);

        //---------------------------------------------------------------------
        // STEP 5: Iterate through files and perform renaming
        //---------------------------------------------------------------------
        // Choose iterator type based on includeSubdirectories flag
        // VARIANT_BOOL is a COM type: VARIANT_TRUE (-1) or VARIANT_FALSE (0)
        if (includeSubdirectories)
        {
            // ===== RECURSIVE ITERATION =====
            // fs::recursive_directory_iterator traverses all subdirectories
            // NOTE: The iterator may throw if it encounters permission issues
            for (const auto& entry : fs::recursive_directory_iterator(dirPath))
            {
                // Skip directories - only process regular files
                if (!fs::is_regular_file(entry.path()))
                    continue;

                filesProcessed++;
                std::string filename = entry.path().filename().string();

                // Check if filename matches the regex filter (e.g., "*.txt")
                if (std::regex_match(filename, fileRegex))
                {
                    // Create a copy of the filename and replace all spaces
                    std::string newFilename = filename;
                    std::replace(newFilename.begin(), newFilename.end(), ' ', replaceCharValue);

                    // Only rename if the filename actually changed
                    if (newFilename != filename)
                    {
                        // Build the new full path: same directory + new filename
                        fs::path newPath = entry.path().parent_path() / newFilename;

                        // Attempt the rename operation (handles file locks, conflicts, etc.)
                        if (FileUtils::SafeRename(entry.path(), newPath, errorMsg))
                        {
                            filesRenamed++;
                            Logger::Info("Renamed: " + filename + " -> " + newFilename);
                        }
                        else
                        {
                            // Rename failed - log it but continue processing other files
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
            // ===== NON-RECURSIVE ITERATION =====
            // fs::directory_iterator only processes files in the specified directory
            // Does NOT descend into subdirectories
            for (const auto& entry : fs::directory_iterator(dirPath))
            {
                // Skip directories - only process regular files
                if (!fs::is_regular_file(entry.path()))
                    continue;

                filesProcessed++;
                std::string filename = entry.path().filename().string();

                // Check if filename matches the regex filter
                if (std::regex_match(filename, fileRegex))
                {
                    // Create a copy of the filename and replace all spaces
                    std::string newFilename = filename;
                    std::replace(newFilename.begin(), newFilename.end(), ' ', replaceCharValue);

                    // Only rename if the filename actually changed
                    if (newFilename != filename)
                    {
                        // Build the new full path: same directory + new filename
                        fs::path newPath = entry.path().parent_path() / newFilename;

                        // Attempt the rename operation
                        if (FileUtils::SafeRename(entry.path(), newPath, errorMsg))
                        {
                            filesRenamed++;
                            Logger::Info("Renamed: " + filename + " -> " + newFilename);
                        }
                        else
                        {
                            // Rename failed - log it but continue processing other files
                            errors++;
                            messages.push_back("Failed to rename '" + filename + "': " + errorMsg);
                            Logger::Warning("Failed to rename '" + filename + "': " + errorMsg);
                        }
                    }
                }
            }
        }

        //---------------------------------------------------------------------
        // STEP 6: Build result message for caller
        //---------------------------------------------------------------------
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

//=============================================================================
// CFileManagement::OrganizeDirectory
//=============================================================================
// Organizes files in a directory into subdirectories based on various criteria.
//
// SUPPORTED CRITERIA:
//   - "extension" or "type": Group files by file extension (.txt, .jpg, etc.)
//   - "date" or "modified": Group by modification date (creates YYYY-MM folders)
//   - "size": Group by file size (small < 1MB, medium < 10MB, large >= 10MB)
//   - Any other string: Creates a single folder with that name
//
// ALGORITHM:
//   1. Validate input parameters (directory exists, criteria specified)
//   2. Iterate through files in the directory (non-recursive)
//   3. For each file:
//      a. Determine target subfolder based on criteria
//      b. Create the target subfolder if it doesn't exist
//      c. Move the file into the subfolder
//   4. Build result message with statistics and error details
//   5. Return S_OK or error HRESULT
//
// NOTES:
//   - This operation is NOT recursive - only processes the specified directory
//   - Creates subdirectories automatically as needed
//   - Individual file failures don't stop the overall operation
//
//=============================================================================
STDMETHODIMP CFileManagement::OrganizeDirectory(BSTR directoryPath, BSTR criteria, BSTR* result)
{
    // Statistics tracking
    std::vector<std::string> messages;  // Error messages for detailed reporting
    int filesProcessed = 0;              // Total files examined
    int filesOrganized = 0;              // Successfully moved files
    int errors = 0;                      // Count of failures

    try
    {
        //---------------------------------------------------------------------
        // Parameter Validation (similar pattern to BatchRenameFiles)
        //---------------------------------------------------------------------
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
