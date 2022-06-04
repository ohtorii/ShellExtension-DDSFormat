#pragma once
#include <stdarg.h>
#include <functional>

namespace dds_format {
    class Logger {
    public:
        /// <summary>
        /// <para>ログ出力を行う関数を設定する。</para>
        /// <para>nullptrを指定するとログ出力を行わない(NullLoggerとなる)</para>
        /// </summary>
        /// <param name="writer"></param>
        static void SetRawWriter(std::function<void(const wchar_t*)> writer);

        static void Write(const wchar_t* format, ...);
        static void WriteLine(const wchar_t* format, ...);
    private:
        static void Print(const wchar_t* format, va_list& args);
        static void RawWrite(const wchar_t* message);
        static bool IsNullLogger();
        static std::function<void(const wchar_t*)> m_RawWriter;
    };

    /// <summary>
    /// <para>ログをOutputDebugStringへ出力する</para>
    /// <para>(e.g.) Logger.SetRawWriter(LoggerOutputDebugString::Writer)</para>
    /// </summary>
    class LoggerOutputDebugString {
    public:
        static void Write(const wchar_t *message);
    };
};
