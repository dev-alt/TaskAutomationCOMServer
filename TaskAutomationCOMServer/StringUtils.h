#pragma once
#include <string>
#include <comdef.h>
#include <atlbase.h>
#include <atlstr.h>
#include <sstream>
#include <vector>

namespace StringUtils
{
    // Convert std::string to BSTR (caller must free with SysFreeString)
    inline BSTR StringToBSTR(const std::string& str)
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        if (len == 0)
            return nullptr;

        BSTR bstr = SysAllocStringLen(nullptr, len - 1);
        if (bstr)
        {
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, bstr, len);
        }
        return bstr;
    }

    // Convert std::wstring to BSTR (caller must free with SysFreeString)
    inline BSTR WStringToBSTR(const std::wstring& wstr)
    {
        return SysAllocString(wstr.c_str());
    }

    // Convert BSTR to std::string
    inline std::string BSTRToString(BSTR bstr)
    {
        if (!bstr)
            return "";

        CW2A converter(bstr);
        return std::string(converter);
    }

    // Convert BSTR to std::wstring
    inline std::wstring BSTRToWString(BSTR bstr)
    {
        if (!bstr)
            return L"";

        return std::wstring(bstr);
    }

    // Check if BSTR is null or empty
    inline bool IsNullOrEmpty(BSTR bstr)
    {
        return (bstr == nullptr || SysStringLen(bstr) == 0);
    }

    // Format a string with arguments
    template<typename... Args>
    inline std::string Format(const std::string& format, Args... args)
    {
        int size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
        if (size <= 0)
            return "";

        std::vector<char> buf(size);
        std::snprintf(buf.data(), size, format.c_str(), args...);
        return std::string(buf.data(), buf.data() + size - 1);
    }

    // Build a result message with multiple lines
    inline std::string BuildResultMessage(const std::vector<std::string>& lines)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < lines.size(); ++i)
        {
            oss << lines[i];
            if (i < lines.size() - 1)
                oss << "\n";
        }
        return oss.str();
    }

    // Escape characters in a string for safe output
    inline std::string Escape(const std::string& str)
    {
        std::string result;
        result.reserve(str.length());

        for (char c : str)
        {
            switch (c)
            {
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
            }
        }
        return result;
    }
}
