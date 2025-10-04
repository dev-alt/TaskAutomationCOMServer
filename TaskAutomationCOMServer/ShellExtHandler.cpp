#include "pch.h"
#include "ShellExtHandler.h"
#include "Logger.h"
#include <shlwapi.h>
#include <strsafe.h>

#pragma comment(lib, "shlwapi.lib")

STDMETHODIMP CShellExtHandler::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hkeyProgID)
{
    Logger::Info("ShellExtHandler::Initialize called");

    m_selectedFiles.clear();
    m_selectedFolder.clear();

    if (!pDataObj)
    {
        Logger::Warning("ShellExtHandler::Initialize - No data object provided");
        return E_INVALIDARG;
    }

    // Get the file/folder selection from the data object
    FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stg = { TYMED_HGLOBAL };

    if (FAILED(pDataObj->GetData(&fmt, &stg)))
    {
        Logger::Warning("ShellExtHandler::Initialize - Failed to get data");
        return E_FAIL;
    }

    // Get the file names from the data object
    HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
    if (hDrop)
    {
        UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);

        for (UINT i = 0; i < fileCount; i++)
        {
            wchar_t szFile[MAX_PATH];
            if (DragQueryFile(hDrop, i, szFile, ARRAYSIZE(szFile)))
            {
                m_selectedFiles.push_back(szFile);

                // If it's a directory, save it as the selected folder
                if (PathIsDirectory(szFile))
                {
                    m_selectedFolder = szFile;
                }
            }
        }

        GlobalUnlock(stg.hGlobal);
    }

    ReleaseStgMedium(&stg);

    Logger::Info("ShellExtHandler::Initialize - " + std::to_string(m_selectedFiles.size()) + " items selected");
    return S_OK;
}

STDMETHODIMP CShellExtHandler::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    Logger::Info("ShellExtHandler::QueryContextMenu called");

    // If the flags include CMF_DEFAULTONLY, we shouldn't add menu items
    if (uFlags & CMF_DEFAULTONLY)
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
    }

    // Only show menu for directories or multiple files
    if (m_selectedFiles.empty())
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
    }

    UINT idCmd = idCmdFirst;

    // Create a submenu for our commands
    HMENU hSubmenu = CreatePopupMenu();
    if (!hSubmenu)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Add menu items to the submenu
    InsertMenu(hSubmenu, 0, MF_BYPOSITION | MF_STRING, idCmd + IDM_BATCH_RENAME, L"Batch Rename Files");
    InsertMenu(hSubmenu, 1, MF_BYPOSITION | MF_STRING, idCmd + IDM_ORGANIZE_DIR, L"Organize Directory");

    // Add the submenu to the context menu
    InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_POPUP | MF_STRING,
               reinterpret_cast<UINT_PTR>(hSubmenu), L"Task Automation");

    Logger::Info("ShellExtHandler::QueryContextMenu - Added menu items");

    // Return number of menu items added
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 2);
}

STDMETHODIMP CShellExtHandler::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    Logger::Info("ShellExtHandler::InvokeCommand called");

    // Check if command is from a menu click (not a verb)
    if (HIWORD(pici->lpVerb))
    {
        Logger::Warning("ShellExtHandler::InvokeCommand - Verb commands not supported");
        return E_INVALIDARG;
    }

    // Get the command index
    UINT idCmd = LOWORD(pici->lpVerb);

    switch (idCmd)
    {
    case IDM_BATCH_RENAME:
        Logger::Info("ShellExtHandler::InvokeCommand - Executing Batch Rename");
        ExecuteBatchRename();
        break;

    case IDM_ORGANIZE_DIR:
        Logger::Info("ShellExtHandler::InvokeCommand - Executing Organize Directory");
        ExecuteOrganizeDirectory();
        break;

    default:
        Logger::Warning("ShellExtHandler::InvokeCommand - Unknown command: " + std::to_string(idCmd));
        return E_INVALIDARG;
    }

    return S_OK;
}

STDMETHODIMP CShellExtHandler::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, CHAR* pszName, UINT cchMax)
{
    // Provide help text for menu items
    if (uType == GCS_HELPTEXTA)
    {
        switch (idCmd)
        {
        case IDM_BATCH_RENAME:
            StringCchCopyA(pszName, cchMax, "Batch rename files in the selected directory");
            return S_OK;

        case IDM_ORGANIZE_DIR:
            StringCchCopyA(pszName, cchMax, "Organize files in the directory by type, date, or size");
            return S_OK;
        }
    }
    else if (uType == GCS_HELPTEXTW)
    {
        switch (idCmd)
        {
        case IDM_BATCH_RENAME:
            StringCchCopyW(reinterpret_cast<wchar_t*>(pszName), cchMax,
                          L"Batch rename files in the selected directory");
            return S_OK;

        case IDM_ORGANIZE_DIR:
            StringCchCopyW(reinterpret_cast<wchar_t*>(pszName), cchMax,
                          L"Organize files in the directory by type, date, or size");
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

bool CShellExtHandler::GetSelectedDirectory()
{
    if (!m_selectedFolder.empty())
    {
        return true;
    }

    // If no folder selected, check if we have files and use their parent directory
    if (!m_selectedFiles.empty())
    {
        wchar_t szDir[MAX_PATH];
        StringCchCopy(szDir, ARRAYSIZE(szDir), m_selectedFiles[0].c_str());
        PathRemoveFileSpec(szDir);
        m_selectedFolder = szDir;
        return true;
    }

    return false;
}

void CShellExtHandler::ExecuteBatchRename()
{
    if (!GetSelectedDirectory())
    {
        MessageBox(nullptr, L"No directory selected", L"Task Automation", MB_OK | MB_ICONERROR);
        return;
    }

    // Simple dialog for demonstration
    // In production, you'd want a proper dialog with input fields
    int result = MessageBox(nullptr,
                           L"This will rename all files in the directory by replacing spaces with underscores.\n\n"
                           L"Continue?",
                           L"Batch Rename Files",
                           MB_YESNO | MB_ICONQUESTION);

    if (result == IDYES)
    {
        // Create instance of FileManagement COM object
        CComPtr<IFileManagement> pFileManagement;
        HRESULT hr = pFileManagement.CoCreateInstance(CLSID_FileManagement);

        if (SUCCEEDED(hr))
        {
            CComBSTR result;
            hr = pFileManagement->BatchRenameFiles(
                CComBSTR(m_selectedFolder.c_str()),
                CComBSTR(L"_"),
                VARIANT_FALSE,
                CComBSTR(L".*"),
                &result);

            if (SUCCEEDED(hr))
            {
                MessageBox(nullptr, result, L"Batch Rename Complete", MB_OK | MB_ICONINFORMATION);
                Logger::Info("Batch rename completed successfully");
            }
            else
            {
                MessageBox(nullptr, result ? result.m_str : L"Unknown error",
                          L"Batch Rename Failed", MB_OK | MB_ICONERROR);
                Logger::Error("Batch rename failed");
            }
        }
        else
        {
            MessageBox(nullptr, L"Failed to create FileManagement COM object",
                      L"Error", MB_OK | MB_ICONERROR);
            Logger::Error("Failed to create FileManagement COM object");
        }
    }
}

void CShellExtHandler::ExecuteOrganizeDirectory()
{
    if (!GetSelectedDirectory())
    {
        MessageBox(nullptr, L"No directory selected", L"Task Automation", MB_OK | MB_ICONERROR);
        return;
    }

    // Simple dialog for demonstration
    // In production, you'd want a proper dialog to select organization criteria
    int result = MessageBox(nullptr,
                           L"This will organize files in the directory by file extension.\n\n"
                           L"Continue?",
                           L"Organize Directory",
                           MB_YESNO | MB_ICONQUESTION);

    if (result == IDYES)
    {
        // Create instance of FileManagement COM object
        CComPtr<IFileManagement> pFileManagement;
        HRESULT hr = pFileManagement.CoCreateInstance(CLSID_FileManagement);

        if (SUCCEEDED(hr))
        {
            CComBSTR result;
            hr = pFileManagement->OrganizeDirectory(
                CComBSTR(m_selectedFolder.c_str()),
                CComBSTR(L"extension"),
                &result);

            if (SUCCEEDED(hr))
            {
                MessageBox(nullptr, result, L"Organization Complete", MB_OK | MB_ICONINFORMATION);
                Logger::Info("Directory organization completed successfully");
            }
            else
            {
                MessageBox(nullptr, result ? result.m_str : L"Unknown error",
                          L"Organization Failed", MB_OK | MB_ICONERROR);
                Logger::Error("Directory organization failed");
            }
        }
        else
        {
            MessageBox(nullptr, L"Failed to create FileManagement COM object",
                      L"Error", MB_OK | MB_ICONERROR);
            Logger::Error("Failed to create FileManagement COM object");
        }
    }
}
