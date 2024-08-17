#include "pch.h"
#include "FileManagement.h"
#include <filesystem>
#include <string>
#include <regex>
#include <atlstr.h>

namespace fs = std::filesystem;

STDMETHODIMP CFileManagement::BatchRenameFiles(BSTR directoryPath, BSTR replaceChar, BOOL includeSubdirectories, BSTR fileTypeFilter)
{
    try
    {
        CW2A dir(directoryPath);
        CW2A replace(replaceChar);
        CW2A filter(fileTypeFilter);

        std::regex fileRegex(filter);

        for (const auto& entry : fs::recursive_directory_iterator(dir.m_psz))
        {
            if (fs::is_regular_file(entry.path()))
            {
                std::string filename = entry.path().filename().string();
                if (std::regex_match(filename, fileRegex))
                {
                    std::replace(filename.begin(), filename.end(), ' ', replace.m_psz[0]);
                    fs::rename(entry.path(), entry.path().parent_path() / filename);
                }
            }

            if (!includeSubdirectories && entry.depth() > 0)
                break;
        }

        return S_OK;
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }
}

STDMETHODIMP CFileManagement::OrganizeDirectory(BSTR directoryPath, BSTR criteria)
{
    try
    {
        CW2A dir(directoryPath);
        CW2A crit(criteria);

        for (const auto& entry : fs::directory_iterator(dir.m_psz))
        {
            if (fs::is_regular_file(entry.path()))
            {
                std::string targetDir = entry.path().parent_path().string() + "\\" + crit.m_psz;
                fs::create_directories(targetDir);

                fs::rename(entry.path(), targetDir + "\\" + entry.path().filename().string());
            }
        }

        return S_OK;
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }
}