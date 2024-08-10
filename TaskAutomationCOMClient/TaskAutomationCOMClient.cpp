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
        CoUninitialize();
        return 1;
    }
    if (SUCCEEDED(hr))
    {
        CComBSTR result;
        hr = pFileManagement->BatchRenameFiles(CComBSTR("C:\\TestFolder"),
            CComBSTR("_"),
            VARIANT_TRUE,
            CComBSTR("*.*"),
            &result);
        if (SUCCEEDED(hr))
        {
            std::wcout << "Result: " << result << '\n';
        }
        else
        {
            std::cout << "Method call failed" << '\n';
        }
    }
    else
    {
        std::cout << "Failed to create instance" << '\n';
    }

    CoUninitialize();
    return 0;
}