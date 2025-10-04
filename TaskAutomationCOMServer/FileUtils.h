#pragma once
#include <filesystem>
#include <string>
#include <regex>
#include <system_error>

namespace fs = std::filesystem;

namespace FileUtils
{
    // Check if a path exists and is accessible
    inline bool PathExists(const std::string& path)
    {
        try
        {
            return fs::exists(path);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    // Check if a path is a directory
    inline bool IsDirectory(const std::string& path)
    {
        try
        {
            return fs::is_directory(path);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    // Check if a path is a regular file
    inline bool IsRegularFile(const std::string& path)
    {
        try
        {
            return fs::is_regular_file(path);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    // Validate that a directory exists and is accessible
    inline bool ValidateDirectory(const std::string& path, std::string& errorMessage)
    {
        if (path.empty())
        {
            errorMessage = "Directory path is empty";
            return false;
        }

        if (!PathExists(path))
        {
            errorMessage = "Directory does not exist: " + path;
            return false;
        }

        if (!IsDirectory(path))
        {
            errorMessage = "Path is not a directory: " + path;
            return false;
        }

        return true;
    }

    // Validate a regex pattern
    inline bool ValidateRegexPattern(const std::string& pattern, std::string& errorMessage)
    {
        if (pattern.empty())
        {
            errorMessage = "Regex pattern is empty";
            return false;
        }

        try
        {
            std::regex test(pattern);
            return true;
        }
        catch (const std::regex_error& e)
        {
            errorMessage = "Invalid regex pattern: " + std::string(e.what());
            return false;
        }
    }

    // Get file extension in lowercase
    inline std::string GetExtension(const fs::path& filePath)
    {
        std::string ext = filePath.extension().string();
        if (!ext.empty() && ext[0] == '.')
        {
            ext = ext.substr(1);
        }

        // Convert to lowercase
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

    // Get file size in bytes
    inline uintmax_t GetFileSize(const fs::path& filePath)
    {
        try
        {
            return fs::file_size(filePath);
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    // Get last write time as time_t
    inline std::time_t GetLastWriteTime(const fs::path& filePath)
    {
        try
        {
            auto ftime = fs::last_write_time(filePath);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            return std::chrono::system_clock::to_time_t(sctp);
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    // Safe file rename with error checking
    inline bool SafeRename(const fs::path& oldPath, const fs::path& newPath, std::string& errorMessage)
    {
        try
        {
            // Check if source exists
            if (!fs::exists(oldPath))
            {
                errorMessage = "Source file does not exist: " + oldPath.string();
                return false;
            }

            // Check if destination already exists
            if (fs::exists(newPath))
            {
                errorMessage = "Destination file already exists: " + newPath.string();
                return false;
            }

            fs::rename(oldPath, newPath);
            return true;
        }
        catch (const fs::filesystem_error& e)
        {
            errorMessage = "Failed to rename file: " + std::string(e.what());
            return false;
        }
        catch (const std::exception& e)
        {
            errorMessage = "Unexpected error during rename: " + std::string(e.what());
            return false;
        }
    }

    // Safe directory creation with error checking
    inline bool SafeCreateDirectories(const fs::path& path, std::string& errorMessage)
    {
        try
        {
            if (fs::exists(path))
            {
                if (!fs::is_directory(path))
                {
                    errorMessage = "Path exists but is not a directory: " + path.string();
                    return false;
                }
                return true; // Already exists
            }

            fs::create_directories(path);
            return true;
        }
        catch (const fs::filesystem_error& e)
        {
            errorMessage = "Failed to create directory: " + std::string(e.what());
            return false;
        }
        catch (const std::exception& e)
        {
            errorMessage = "Unexpected error during directory creation: " + std::string(e.what());
            return false;
        }
    }

    // Determine organization folder based on criteria
    inline std::string DetermineOrganizationFolder(const fs::path& filePath, const std::string& criteria)
    {
        if (criteria == "extension" || criteria == "type")
        {
            std::string ext = GetExtension(filePath);
            return ext.empty() ? "no_extension" : ext;
        }
        else if (criteria == "date" || criteria == "modified")
        {
            std::time_t time = GetLastWriteTime(filePath);
            std::tm tm;
            localtime_s(&tm, &time);

            char buffer[32];
            strftime(buffer, sizeof(buffer), "%Y-%m", &tm);
            return std::string(buffer);
        }
        else if (criteria == "size")
        {
            uintmax_t size = GetFileSize(filePath);
            if (size < 1024 * 1024) // < 1MB
                return "small";
            else if (size < 10 * 1024 * 1024) // < 10MB
                return "medium";
            else
                return "large";
        }
        else
        {
            // Default: use criteria as folder name
            return criteria;
        }
    }
}
