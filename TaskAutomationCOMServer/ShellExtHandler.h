#pragma once
#include "resource.h"
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>

// Menu command IDs
#define IDM_BATCH_RENAME    0
#define IDM_ORGANIZE_DIR    1

class ATL_NO_VTABLE CShellExtHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CShellExtHandler, &CLSID_ShellExtHandler>,
    public IShellExtInit,
    public IContextMenu
{
public:
    CShellExtHandler() = default;

    DECLARE_REGISTRY_RESOURCEID(IDR_SHELLEXTHANDLER)

    BEGIN_COM_MAP(CShellExtHandler)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IContextMenu)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

    // IShellExtInit methods
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hkeyProgID);

    // IContextMenu methods
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO pici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pReserved, CHAR* pszName, UINT cchMax);

private:
    std::vector<std::wstring> m_selectedFiles;
    std::wstring m_selectedFolder;

    bool GetSelectedDirectory();
    void ExecuteBatchRename();
    void ExecuteOrganizeDirectory();
};

OBJECT_ENTRY_AUTO(CLSID_ShellExtHandler, CShellExtHandler)
