#pragma once
#include<windows.h>

namespace dds_loader_flags {

    size_t GetDDPFFlagsAsWChar(wchar_t* const wcstr, size_t sizeInWords, DWORD flags);
    size_t GetCapsAsWChar(wchar_t* wcstr, size_t sizeInWords, DWORD caps);
    size_t GetCaps2AsWChar(wchar_t* wcstr, size_t sizeInWords, DWORD caps2);
    size_t GetDx10FormatAsWChar(wchar_t* wcstr, size_t sizeInWords, DWORD format);
};

