#pragma once
#include "resource.h"
#include "TaskAutomationCOMServer_i.h"

class ATL_NO_VTABLE CFileManagement :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFileManagement, &CLSID_FileManagement>,
    public IDispatchImpl<IFileManagement, &IID_IFileManagement, &LIBID_TaskAutomationCOMServerLib, 1, 0>
{
public:
    CFileManagement() = default;

    DECLARE_REGISTRY_RESOURCEID(IDR_FILEMANAGEMENT)

    BEGIN_COM_MAP(CFileManagement)
        COM_INTERFACE_ENTRY(IFileManagement)
        COM_INTERFACE_ENTRY(IDispatch)
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
    STDMETHOD(BatchRenameFiles)(BSTR directoryPath, BSTR replaceChar, VARIANT_BOOL includeSubdirectories, BSTR fileTypeFilter, BSTR* result);
    STDMETHOD(OrganizeDirectory)(BSTR directoryPath, BSTR criteria, BSTR* result);

private:
    // Helper methods (if needed)
};

OBJECT_ENTRY_AUTO(__uuidof(FileManagement), CFileManagement)