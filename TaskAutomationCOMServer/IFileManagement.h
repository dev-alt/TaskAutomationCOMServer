#pragma once
#include <windows.h>
#include <objbase.h>

// {Unique GUID for IFileManagement interface}
const IID IID_IFileManagement =
{ 0xB68C9A85, 0x6256, 0x45F2, {0xA6, 0xA2, 0x6A, 0x22, 0x35, 0x6F, 0x51, 0x79} };

class IFileManagement : public IUnknown
{
public:
    virtual HRESULT __stdcall BatchRenameFiles(
        BSTR directoryPath,
        BSTR replaceChar,
        BOOL includeSubdirectories,
        BSTR fileTypeFilter) = 0;

    virtual HRESULT __stdcall OrganizeDirectory(
        BSTR directoryPath,
        BSTR criteria) = 0;
};