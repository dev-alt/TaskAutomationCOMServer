#include <iostream>
#include <Windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include "../TaskAutomationCOMServer/TaskAutomationCOMServer_i.h"
#include "../TaskAutomationCOMServer/TaskAutomationCOMServer_i.c"

#include <comdef.h>

int main()
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cout << "Failed to initialize COM library. Error code: " << hr << '\n';
        return 1;
    }

    CComPtr<IFileManagement> pFileManagement;
    hr = pFileManagement.CoCreateInstance(CLSID_FileManagement);

    if (FAILED(hr)) {
        _com_error err(hr);
        std::wcout << L"Failed to create instance. Error message: " << err.ErrorMessage() << '\n';
        std::cout << "HRESULT: " << std::hex << hr << std::dec << '\n';
        std::cout << "\nMake sure the COM server is registered. Run:\n";
        std::cout << "  regsvr32 TaskAutomationCOMServer.dll\n";
        CoUninitialize();
        return 1;
    }

    std::cout << "COM Server instance created successfully!\n\n";

    // Test BatchRenameFiles
    std::cout << "=== Testing BatchRenameFiles ===\n";
    CComBSTR result;
    hr = pFileManagement->BatchRenameFiles(
        CComBSTR("C:\\TestFolder"),
        CComBSTR("_"),
        VARIANT_TRUE,
        CComBSTR(".*\\.txt"),
        &result);

    if (SUCCEEDED(hr))
    {
        std::wcout << L"BatchRenameFiles Result:\n" << result.m_str << L"\n\n";
    }
    else
    {
        _com_error err(hr);
        std::wcout << L"BatchRenameFiles failed. Error: " << err.ErrorMessage() << L"\n";
        if (result.m_str != nullptr)
        {
            std::wcout << L"Details: " << result.m_str << L"\n\n";
        }
    }

    // Test OrganizeDirectory
    std::cout << "=== Testing OrganizeDirectory ===\n";
    result.Empty();
    hr = pFileManagement->OrganizeDirectory(
        CComBSTR("C:\\TestFolder"),
        CComBSTR("extension"),
        &result);

    if (SUCCEEDED(hr))
    {
        std::wcout << L"OrganizeDirectory Result:\n" << result.m_str << L"\n\n";
    }
    else
    {
        _com_error err(hr);
        std::wcout << L"OrganizeDirectory failed. Error: " << err.ErrorMessage() << L"\n";
        if (result.m_str != nullptr)
        {
            std::wcout << L"Details: " << result.m_str << L"\n\n";
        }
    }

    std::cout << "Tests completed. Press Enter to exit...";
    std::cin.get();

    CoUninitialize();
    return 0;
}
