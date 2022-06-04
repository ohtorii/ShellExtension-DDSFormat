#include "pch.h"
#include"logger.h"
#include<type_traits>

namespace dds_format {

    /////////////////////////////////////////////////////////////////////////
    //  Logger
    /////////////////////////////////////////////////////////////////////////
    std::function<void(const wchar_t*)> Logger::m_RawWriter=nullptr;

    bool Logger::IsNullLogger() {
        if (m_RawWriter == nullptr) {
            return true;
        }
        return false;
    }

    void Logger::SetRawWriter(std::function<void(const wchar_t*)> writer) {
        m_RawWriter = writer;
    }

    void Logger::WriteLine(const wchar_t* format, ...) {
        if (IsNullLogger()) {
            return;
        }
        va_list args;
        va_start(args, format);
        Print(format, args);
        va_end(args);
        RawWrite(L"\n");
    }
    void Logger::Write(const wchar_t* format, ...) {
        if (IsNullLogger()) {
            return;
        }
        va_list args;
        va_start(args, format);
        Print(format, args);
        va_end(args);
    }

    void Logger::Print(const wchar_t* format, va_list& args) {
        wchar_t buf[256];
        const auto writeCount = vswprintf_s(buf, _countof(buf), format, args);
        if (0 < writeCount) {
            RawWrite(buf);
        }
    }

    void Logger::RawWrite(const wchar_t* message) {
        if (m_RawWriter != nullptr) {
            m_RawWriter(message);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    //  LoggerOutputDebugString
    /////////////////////////////////////////////////////////////////////////
    void LoggerOutputDebugString::Write(const wchar_t *message){
        OutputDebugStringW(message);
    }

};

