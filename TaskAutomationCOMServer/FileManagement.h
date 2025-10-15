#pragma once
#include "resource.h"
#include "TaskAutomationCOMServer_i.h"

//=============================================================================
// CFileManagement
//=============================================================================
//
// PURPOSE:
//   ATL-based COM component that provides file management automation services.
//   This class implements the IFileManagement interface defined in the IDL file
//   and provides methods for batch file operations like renaming and organizing.
//
// COM ARCHITECTURE:
//   - CComObjectRootEx<CComSingleThreadModel>:
//       Provides reference counting and IUnknown implementation.
//       Uses single-threaded model (no thread synchronization overhead).
//
//   - CComCoClass<CFileManagement, &CLSID_FileManagement>:
//       Provides class factory functionality and CLSID association.
//       The CLSID (Class ID) uniquely identifies this COM class in the registry.
//
//   - IDispatchImpl<IFileManagement, ...>:
//       Implements IDispatch for automation support (allows scripting languages
//       like VBScript/JScript to call methods). The "dual" interface in IDL
//       means this supports both vtable binding (C++) and IDispatch (scripting).
//
// THREADING MODEL:
//   Single-threaded apartment (STA). All calls to this object must come from
//   the same thread that created it, or COM will marshal calls automatically.
//
// REGISTRATION:
//   DECLARE_REGISTRY_RESOURCEID associates this class with an .rgs registry
//   script resource that defines how the COM object is registered in Windows.
//
//=============================================================================

class ATL_NO_VTABLE CFileManagement :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFileManagement, &CLSID_FileManagement>,
    public IDispatchImpl<IFileManagement, &IID_IFileManagement, &LIBID_TaskAutomationCOMServerLib, 1, 0>
{
public:
    // Default constructor - COM objects must have default constructors
    CFileManagement() = default;

    // Associates this class with a registry script resource (IDR_FILEMANAGEMENT)
    // that contains instructions for registering this COM object in the Windows registry
    DECLARE_REGISTRY_RESOURCEID(IDR_FILEMANAGEMENT)

    // COM_MAP defines which interfaces this object exposes to clients
    // When a client calls QueryInterface for IFileManagement or IDispatch,
    // ATL uses this map to return the correct interface pointer
    BEGIN_COM_MAP(CFileManagement)
        COM_INTERFACE_ENTRY(IFileManagement)  // Primary custom interface
        COM_INTERFACE_ENTRY(IDispatch)        // Automation interface for scripting
    END_COM_MAP()

    // Protects FinalConstruct/FinalRelease from being called on partially
    // constructed or destructed objects during exception handling
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // FinalConstruct is called after the object is fully constructed and
    // reference count is initialized. Use this for initialization that might fail.
    // Return E_FAIL or other error HRESULT to prevent object creation.
    HRESULT FinalConstruct()
    {
        return S_OK;  // No special initialization needed
    }

    // FinalRelease is called just before the object is destroyed (ref count = 0).
    // Use this to clean up resources, close handles, release COM pointers, etc.
    void FinalRelease()
    {
        // No cleanup needed for this simple implementation
    }

    //=========================================================================
    // IFileManagement Interface Methods
    //=========================================================================
    // These methods implement the interface defined in TaskAutomationCOMServer.idl
    // All COM interface methods must return HRESULT (S_OK for success, E_* for errors)
    //=========================================================================

    //-------------------------------------------------------------------------
    // BatchRenameFiles
    //-------------------------------------------------------------------------
    // Renames files in a directory by replacing spaces with a specified character.
    //
    // PARAMETERS:
    //   directoryPath - [in] Full path to directory containing files to rename
    //   replaceChar - [in] Character to replace spaces with (e.g., "_", "-")
    //   includeSubdirectories - [in] If TRUE, recursively process subdirectories
    //   fileTypeFilter - [in] Regex pattern to filter files (e.g., ".*\.txt" for text files)
    //   result - [out, retval] BSTR containing operation summary and any errors
    //
    // RETURNS:
    //   S_OK - Operation completed (check result string for details)
    //   E_INVALIDARG - Invalid parameter (empty path, invalid regex, etc.)
    //   E_POINTER - result parameter is NULL
    //   E_FAIL - Unexpected error during operation
    //
    // NOTES:
    //   - Caller must free the returned BSTR using SysFreeString()
    //   - Operation continues even if some files fail (partial success possible)
    //   - All errors are logged and included in the result string
    //-------------------------------------------------------------------------
    STDMETHOD(BatchRenameFiles)(
        BSTR directoryPath,
        BSTR replaceChar,
        VARIANT_BOOL includeSubdirectories,
        BSTR fileTypeFilter,
        BSTR* result);

    //-------------------------------------------------------------------------
    // OrganizeDirectory
    //-------------------------------------------------------------------------
    // Organizes files in a directory into subdirectories based on criteria.
    //
    // PARAMETERS:
    //   directoryPath - [in] Full path to directory to organize
    //   criteria - [in] Organization method:
    //                    "extension" or "type" - Group by file extension
    //                    "date" or "modified" - Group by modification date (YYYY-MM)
    //                    "size" - Group by file size (small/medium/large)
    //                    Other - Create single folder with criteria as name
    //   result - [out, retval] BSTR containing operation summary and any errors
    //
    // RETURNS:
    //   S_OK - Operation completed (check result string for details)
    //   E_INVALIDARG - Invalid parameter (empty path, invalid criteria, etc.)
    //   E_POINTER - result parameter is NULL
    //   E_FAIL - Unexpected error during operation
    //
    // NOTES:
    //   - Caller must free the returned BSTR using SysFreeString()
    //   - Only processes files in the specified directory (not recursive)
    //   - Automatically creates target subdirectories as needed
    //   - Operation continues even if some files fail (partial success possible)
    //-------------------------------------------------------------------------
    STDMETHOD(OrganizeDirectory)(
        BSTR directoryPath,
        BSTR criteria,
        BSTR* result);

private:
    // Future: Add helper methods here as the class grows
    // Example: ValidateInputs(), BuildResultString(), etc.
};

// OBJECT_ENTRY_AUTO registers this class in ATL's object map
// This allows ATL to create instances of this class when clients call
// CoCreateInstance(CLSID_FileManagement, ...) or use class factories
OBJECT_ENTRY_AUTO(__uuidof(FileManagement), CFileManagement)