#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <windows.h>

namespace Logger
{
    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error
    };

    class FileLogger
    {
    private:
        std::ofstream logFile;
        std::mutex logMutex;
        LogLevel minLevel;
        bool enabled;

        std::string GetTimestamp()
        {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            std::tm tm;
            localtime_s(&tm, &time);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
            oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return oss.str();
        }

        std::string LevelToString(LogLevel level)
        {
            switch (level)
            {
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error:   return "ERROR";
            default:                return "UNKNOWN";
            }
        }

    public:
        FileLogger(const std::string& filename = "", LogLevel minLevel = LogLevel::Info)
            : minLevel(minLevel), enabled(false)
        {
            if (!filename.empty())
            {
                logFile.open(filename, std::ios::app);
                enabled = logFile.is_open();
            }
        }

        ~FileLogger()
        {
            if (logFile.is_open())
            {
                logFile.close();
            }
        }

        void SetEnabled(bool enable)
        {
            enabled = enable;
        }

        void SetMinLevel(LogLevel level)
        {
            minLevel = level;
        }

        void Log(LogLevel level, const std::string& message)
        {
            if (!enabled || level < minLevel)
                return;

            std::lock_guard<std::mutex> lock(logMutex);

            std::string logEntry = GetTimestamp() + " [" + LevelToString(level) + "] " + message;

            if (logFile.is_open())
            {
                logFile << logEntry << std::endl;
                logFile.flush();
            }

            // Also output to debugger
#ifdef _DEBUG
            OutputDebugStringA((logEntry + "\n").c_str());
#endif
        }

        void Debug(const std::string& message)
        {
            Log(LogLevel::Debug, message);
        }

        void Info(const std::string& message)
        {
            Log(LogLevel::Info, message);
        }

        void Warning(const std::string& message)
        {
            Log(LogLevel::Warning, message);
        }

        void Error(const std::string& message)
        {
            Log(LogLevel::Error, message);
        }
    };

    // Global logger instance
    inline FileLogger& GetInstance()
    {
        static FileLogger instance;
        return instance;
    }

    // Convenience functions
    inline void Debug(const std::string& message)
    {
        GetInstance().Debug(message);
    }

    inline void Info(const std::string& message)
    {
        GetInstance().Info(message);
    }

    inline void Warning(const std::string& message)
    {
        GetInstance().Warning(message);
    }

    inline void Error(const std::string& message)
    {
        GetInstance().Error(message);
    }

    inline void SetEnabled(bool enable)
    {
        GetInstance().SetEnabled(enable);
    }

    inline void SetMinLevel(LogLevel level)
    {
        GetInstance().SetMinLevel(level);
    }
}
