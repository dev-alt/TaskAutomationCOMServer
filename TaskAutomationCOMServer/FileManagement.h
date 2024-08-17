#pragma once
#include "IFileManagement.h"
#include <atlbase.h>
#include <atlcom.h>

class ATL_NO_VTABLE CFileManagement :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFileManagement, &CLSID_FileManagement>,
    public IFileManagement
{
public:
    CFileManagement() = default;

    DECLARE_REGISTRY_RESOURCEID(IDR_FILEMANAGEMENT)

    BEGIN_COM_MAP(CFileManagement)
        COM_INTERFACE_ENTRY(IFileManagement)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

    // IFileManagement methods
    STDMETHOD(BatchRenameFiles)(BSTR directoryPath, BSTR replaceChar, BOOL includeSubdirectories, BSTR fileTypeFilter) override;
    STDMETHOD(OrganizeDirectory)(BSTR directoryPath, BSTR criteria) override;

private:
    // Helper methods (if needed)
};

OBJECT_ENTRY_AUTO(__uuidof(FileManagement), CFileManagement)