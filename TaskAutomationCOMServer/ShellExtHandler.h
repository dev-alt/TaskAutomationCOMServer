#pragma once
#include "resource.h"
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>

//=============================================================================
// CShellExtHandler
//=============================================================================
//
// PURPOSE:
//   Windows Shell Extension that adds custom context menu items to Windows Explorer.
//   When users right-click on files/folders, this extension adds a "Task Automation"
//   submenu with file management operations.
//
// SHELL EXTENSION CONCEPTS:
//   - IShellExtInit: Initializes the extension with information about the selected
//                    files/folders when the user right-clicks in Explorer.
//
//   - IContextMenu: Provides methods to add menu items, handle menu clicks, and
//                   provide help text for the context menu.
//
// REGISTRATION:
//   Shell extensions must be registered in the Windows Registry under:
//   HKCR\*\shellex\ContextMenuHandlers\{CLSID}        - For files
//   HKCR\Directory\shellex\ContextMenuHandlers\{CLSID} - For folders
//
// SECURITY:
//   Shell extensions run in-process with explorer.exe, so crashes affect Explorer.
//   Always validate inputs and handle exceptions carefully!
//
// THREADING:
//   Shell extensions can be loaded on any thread. Use CComSingleThreadModel
//   with apartment threading, or CComMultiThreadModel for free threading.
//
//=============================================================================

//-----------------------------------------------------------------------------
// Menu Command IDs
//-----------------------------------------------------------------------------
// These identify which menu item was clicked when InvokeCommand is called.
// Must be unique within our extension (relative to idCmdFirst).
// IMPORTANT: When adding new menu items, add new IDM_* constants here.
//-----------------------------------------------------------------------------
#define IDM_BATCH_RENAME            0
#define IDM_ORGANIZE_DIR            1
#define IDM_DELETE_EMPTY_FOLDERS    2
#define IDM_FIND_DUPLICATES         3
#define IDM_CONVERT_TO_LOWERCASE    4
#define IDM_CONVERT_TO_UPPERCASE    5
#define IDM_REMOVE_SPECIAL_CHARS    6
#define IDM_FILE_SIZE_REPORT        7

// Total number of menu items (update when adding new commands)
#define TOTAL_MENU_ITEMS            8

class ATL_NO_VTABLE CShellExtHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CShellExtHandler, &CLSID_ShellExtHandler>,
    public IShellExtInit,      // Required: Receives selection info from Explorer
    public IContextMenu        // Required: Adds items to context menu
{
public:
    CShellExtHandler() = default;

    // Registry resource ID for shell extension registration script
    DECLARE_REGISTRY_RESOURCEID(IDR_SHELLEXTHANDLER)

    // COM interface map - exposes IShellExtInit and IContextMenu to Explorer
    BEGIN_COM_MAP(CShellExtHandler)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IContextMenu)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Called after object construction - use for initialization
    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    // Called before object destruction - use for cleanup
    void FinalRelease()
    {
        // Clear any stored file paths
        m_selectedFiles.clear();
        m_selectedFolder.clear();
    }

    //=========================================================================
    // IShellExtInit Interface
    //=========================================================================
    // Windows Explorer calls this method when the user right-clicks on items.
    // This gives us information about what was selected.
    //=========================================================================

    //-------------------------------------------------------------------------
    // Initialize
    //-------------------------------------------------------------------------
    // Called by Windows Explorer to initialize the shell extension with
    // information about the selected files/folders.
    //
    // PARAMETERS:
    //   pidlFolder - [in] PIDL (pointer to item ID list) of the folder containing
    //                the selected items. Can be NULL if items are from different folders.
    //   pDataObj - [in] IDataObject containing the selected files/folders in
    //              CF_HDROP format (list of file paths). Never NULL.
    //   hkeyProgID - [in] Registry key for file class. Usually not used.
    //
    // RETURNS:
    //   S_OK - Extension initialized successfully
    //   E_INVALIDARG - Invalid parameters
    //   E_FAIL - Initialization failed
    //
    // NOTES:
    //   - This method must complete quickly to avoid slowing down Explorer
    //   - Store selection info in member variables for later use
    //   - If this fails, QueryContextMenu won't be called
    //-------------------------------------------------------------------------
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hkeyProgID);

    //=========================================================================
    // IContextMenu Interface
    //=========================================================================
    // These methods handle the context menu UI and user interactions.
    //=========================================================================

    //-------------------------------------------------------------------------
    // QueryContextMenu
    //-------------------------------------------------------------------------
    // Called by Explorer to let us add items to the context menu.
    //
    // PARAMETERS:
    //   hmenu - [in] Handle to the context menu being built
    //   indexMenu - [in] Zero-based position where we should insert items
    //   idCmdFirst - [in] First command ID we're allowed to use
    //   idCmdLast - [in] Last command ID we're allowed to use
    //   uFlags - [in] Flags controlling menu behavior:
    //            CMF_DEFAULTONLY - Only add default menu item (we add nothing)
    //            CMF_EXPLORE - Menu is for Explorer tree view
    //            CMF_NORMAL - Standard context menu
    //
    // RETURNS:
    //   MAKE_HRESULT(SEVERITY_SUCCESS, 0, numItemsAdded) - Success, returns count
    //   Error HRESULT - Failed to add menu items
    //
    // NOTES:
    //   - Menu items must use IDs in range [idCmdFirst, idCmdLast]
    //   - Return value encodes the number of menu items added
    //   - Should complete quickly to avoid menu display delays
    //-------------------------------------------------------------------------
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);

    //-------------------------------------------------------------------------
    // InvokeCommand
    //-------------------------------------------------------------------------
    // Called when the user clicks on one of our menu items.
    //
    // PARAMETERS:
    //   pici - [in] Pointer to CMINVOKECOMMANDINFO structure containing:
    //          - lpVerb: Either menu command ID (LOWORD) or verb string (HIWORD)
    //          - hwnd: Parent window for any UI dialogs
    //          - lpParameters: Additional parameters (rarely used)
    //
    // RETURNS:
    //   S_OK - Command executed successfully
    //   E_INVALIDARG - Unknown command or invalid parameters
    //   E_FAIL - Command execution failed
    //
    // NOTES:
    //   - This is where we perform the actual file operations
    //   - Can show UI (dialogs, progress windows) to the user
    //   - Long operations should run on background threads
    //   - Always check HIWORD(lpVerb) to distinguish IDs from verb strings
    //-------------------------------------------------------------------------
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO pici);

    //-------------------------------------------------------------------------
    // GetCommandString
    //-------------------------------------------------------------------------
    // Called by Explorer to get help text or verb strings for menu items.
    //
    // PARAMETERS:
    //   idCmd - [in] Command ID (relative to idCmdFirst from QueryContextMenu)
    //   uType - [in] Type of information requested:
    //           GCS_HELPTEXTA/W - Help text shown in Explorer status bar
    //           GCS_VERBA/W - Verb name for command-line invocation
    //           GCS_VALIDATEA/W - Validate the command
    //   pReserved - [in] Reserved, must be NULL
    //   pszName - [out] Buffer to receive the string
    //   cchMax - [in] Size of pszName buffer in characters
    //
    // RETURNS:
    //   S_OK - Information provided successfully
    //   E_INVALIDARG - Unknown command or unsupported info type
    //
    // NOTES:
    //   - Help text appears in Explorer's status bar when hovering over items
    //   - Should be brief and descriptive (max ~80 characters)
    //-------------------------------------------------------------------------
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pReserved, CHAR* pszName, UINT cchMax);

private:
    //=========================================================================
    // Member Variables
    //=========================================================================
    std::vector<std::wstring> m_selectedFiles;  // All selected files/folders from Explorer
    std::wstring m_selectedFolder;               // Primary folder (if a folder was selected)

    //=========================================================================
    // Helper Methods
    //=========================================================================

    //-------------------------------------------------------------------------
    // GetSelectedDirectory
    //-------------------------------------------------------------------------
    // Determines the target directory for operations.
    // If a folder was selected, uses that. Otherwise, uses the parent folder
    // of the first selected file.
    //
    // RETURNS:
    //   true - m_selectedFolder contains a valid directory
    //   false - No directory could be determined
    //-------------------------------------------------------------------------
    bool GetSelectedDirectory();

    //-------------------------------------------------------------------------
    // ExecuteBatchRename
    //-------------------------------------------------------------------------
    // Shows a confirmation dialog, then calls the FileManagement COM object
    // to perform batch file renaming (replacing spaces with underscores).
    //-------------------------------------------------------------------------
    void ExecuteBatchRename();

    //-------------------------------------------------------------------------
    // ExecuteOrganizeDirectory
    //-------------------------------------------------------------------------
    // Shows a confirmation dialog, then calls the FileManagement COM object
    // to organize files by extension into subdirectories.
    //-------------------------------------------------------------------------
    void ExecuteOrganizeDirectory();

    //-------------------------------------------------------------------------
    // ExecuteDeleteEmptyFolders
    //-------------------------------------------------------------------------
    // Scans the selected directory and removes all empty subdirectories.
    // Shows confirmation dialog before deletion.
    //-------------------------------------------------------------------------
    void ExecuteDeleteEmptyFolders();

    //-------------------------------------------------------------------------
    // ExecuteFindDuplicates
    //-------------------------------------------------------------------------
    // Finds duplicate files in the selected directory based on file size
    // and content hash. Shows results in a message box.
    //-------------------------------------------------------------------------
    void ExecuteFindDuplicates();

    //-------------------------------------------------------------------------
    // ExecuteConvertToLowercase
    //-------------------------------------------------------------------------
    // Converts all filenames in the directory to lowercase.
    // Shows confirmation dialog before renaming.
    //-------------------------------------------------------------------------
    void ExecuteConvertToLowercase();

    //-------------------------------------------------------------------------
    // ExecuteConvertToUppercase
    //-------------------------------------------------------------------------
    // Converts all filenames in the directory to UPPERCASE.
    // Shows confirmation dialog before renaming.
    //-------------------------------------------------------------------------
    void ExecuteConvertToUppercase();

    //-------------------------------------------------------------------------
    // ExecuteRemoveSpecialChars
    //-------------------------------------------------------------------------
    // Removes special characters from filenames, keeping only alphanumeric
    // characters, spaces, dots, and underscores.
    //-------------------------------------------------------------------------
    void ExecuteRemoveSpecialChars();

    //-------------------------------------------------------------------------
    // ExecuteFileSizeReport
    //-------------------------------------------------------------------------
    // Generates a report showing total size, file count, and breakdown by
    // file type for the selected directory.
    //-------------------------------------------------------------------------
    void ExecuteFileSizeReport();
};

// Register this class in ATL's object map so it can be instantiated by Explorer
OBJECT_ENTRY_AUTO(CLSID_ShellExtHandler, CShellExtHandler)
