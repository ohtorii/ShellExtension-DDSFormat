#include "DDSLoader.h"
#include "internal/DDSFormatUtils.h"
#include "internal/DDSFormatFlags.h"
#include <bit>

using namespace dds_loader_utils;

namespace {
    constexpr DWORD SIGNATURE = 0x20534444;//"DDS " (Little endian)

    class AutoClose {
    public:
        AutoClose(const AutoClose&) = delete;
        AutoClose& operator=(const AutoClose&) = delete;

        AutoClose(FILE* fp) : m_fp(fp) {
        }
        ~AutoClose() {
            if (m_fp == nullptr) {
                return;
            }
            fclose(m_fp);
            m_fp = nullptr;
        }
    private:
        FILE* m_fp;
    };
};

namespace dds_loader {
    Loader::Loader() {
        Initialize();
    }

    Loader::Loader(const wchar_t* fileName) {
        Initialize();
        Load(fileName);
    }
    void Loader::Initialize() {
        memset(&m_chunk.m_header, 0, sizeof(m_chunk.m_header));
        m_chunk.m_validDDS = false;
        m_chunk.m_isDX10 = false;
    }

    Chunk& Loader::RefChunk() {
        return m_chunk;
    }

    bool Loader::Load(const wchar_t* fileName) {
        FILE* file = nullptr;
        {
            auto error = _wfopen_s(&file, fileName, L"rbS");
            if (error != 0) {
                return false;
            }
            if (file == nullptr) {
                return false;
            }
        }
        AutoClose closer(file);
        {
            const auto readCount = fread_s(&m_chunk.m_header, sizeof(m_chunk.m_header), 1, sizeof(m_chunk.m_header), file);
            if (readCount < sizeof(m_chunk.m_header.dx7)) {
                Initialize();
                return false;
            }
            if (readCount < sizeof(m_chunk.m_header.dx10)) {
                m_chunk.m_isDX10 = false;
            }
            else {
                m_chunk.m_isDX10 = true;
            }
            if (m_chunk.m_header.dx7.dwSignature != SIGNATURE) {
                Initialize();
                return false;
            }
        }
        m_chunk.m_validDDS = true;
        return true;
    }
    DWORD   Loader::GetWidth()const {
        return m_chunk.m_header.dx7.dwWidth;
    }
    DWORD   Loader::GetHeight()const {
        return m_chunk.m_header.dx7.dwHeight;
    }
    bool    Loader::IsWidth2N()const {
        return std::has_single_bit(GetWidth());
    }
    bool    Loader::IsHeight2N()const {
        return std::has_single_bit(GetHeight());
    }
    bool    Loader::IsSquare()const {
        return GetWidth() == GetHeight();
    }

    size_t Loader::GetFourCCAsAsciiDump(wchar_t* wcstr, size_t sizeInWords)const {
        if (m_chunk.m_validDDS) {
            auto const  fourcc = GetFourCC();
            return ConvertByteArrayToWCstrAscii(wcstr, sizeInWords, fourcc.data(), fourcc.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    size_t Loader::GetFourCCAsHexDump(wchar_t* wcstr, size_t sizeInWords)const{
        if (m_chunk.m_validDDS) {
            auto const  fourcc = GetFourCC();
            return ConvertByteArrayToWCstrHex(wcstr, sizeInWords, fourcc.data(), fourcc.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    std::array<BYTE, static_cast<size_t>(Loader::MemberSize::FourCCSize)> Loader::GetFourCC()const {
        BYTE buf[sizeof(DWORD)];
        DWordToByteArray(buf, m_chunk.m_header.dx7.ddspf.dwFourCC);
        return std::array<BYTE, static_cast<size_t>(Loader::MemberSize::FourCCSize)>{buf[0], buf[1], buf[2], buf[3]};
    }

    DWORD Loader::GetMipMapCount()const {
        return m_chunk.m_header.dx7.dwMipMapCount;
    }

    DWORD Loader::GetDepth()const {
        return m_chunk.m_header.dx7.dwDepth;
    }

    DWORD Loader::GetRGBBitCount()const {
        return m_chunk.m_header.dx7.ddspf.dwRGBBitCount;
    }
    DWORD Loader::GetRBitMask()const {
        return m_chunk.m_header.dx7.ddspf.dwRBitMask;
    }
    DWORD Loader::GetGBitMask()const {
        return m_chunk.m_header.dx7.ddspf.dwGBitMask;
    }
    DWORD Loader::GetBBitMask()const {
        return m_chunk.m_header.dx7.ddspf.dwBBitMask;
    }
    DWORD Loader::GetABitMask()const {
        return m_chunk.m_header.dx7.ddspf.dwABitMask;
    }
    size_t Loader::GetRBitMaskAsHexWChar(wchar_t* wcstr, size_t sizeInWords)const {
        return DWordToHexWCStr(wcstr,sizeInWords,GetRBitMask());
    }
    size_t Loader::GetGBitMaskAsHexWChar(wchar_t* wcstr, size_t sizeInWords)const {
        return DWordToHexWCStr(wcstr,sizeInWords,GetGBitMask());
    }
    size_t Loader::GetBBitMaskAsHexWChar(wchar_t* wcstr, size_t sizeInWords)const {
        return DWordToHexWCStr(wcstr,sizeInWords,GetBBitMask());
    }
    size_t Loader::GetABitMaskAsHexWChar(wchar_t* wcstr, size_t sizeInWords)const {
        return DWordToHexWCStr(wcstr,sizeInWords,GetABitMask());
    }
    DWORD Loader::GetCaps()const {
        return m_chunk.m_header.dx7.dwCaps;
    }

    size_t Loader::GetCapsAsWChar(wchar_t* wcstr, size_t sizeInWords)const {
        if (! m_chunk.m_validDDS) {
            return MakeEmptyWStr(wcstr,sizeInWords);
        }
        return dds_loader_flags::GetCapsAsWChar(wcstr, sizeInWords, GetCaps());
    }

    DWORD Loader::GetCaps2()const {
        return m_chunk.m_header.dx7.dwCaps2;
    }

    size_t Loader::GetCaps2AsWChar(wchar_t* wcstr, size_t sizeInWords)const {
        if (! m_chunk.m_validDDS) {
            return MakeEmptyWStr(wcstr,sizeInWords);
        }
        return dds_loader_flags::GetCaps2AsWChar(wcstr, sizeInWords, GetCaps2());
    }

    std::array<BYTE, static_cast<size_t>(Loader::MemberSize::Reserved1Size)> Loader::GetReserved1()const {
        std::array<BYTE, static_cast<size_t>(Loader::MemberSize::Reserved1Size)> result = {0,};
        auto it = result.begin();
        for (const DWORD value : m_chunk.m_header.dx7.dwReserved1) {
            BYTE buf[sizeof(DWORD)];
            DWordToByteArray(buf, value);
            it[0] = buf[0];
            it[1] = buf[1];
            it[2] = buf[2];
            it[3] = buf[3];

            std::advance(it, sizeof(DWORD));
        }
        return result;
    }

    size_t Loader::GetReserved1AsWChar(wchar_t* wcstr, size_t sizeInWords)const {
        if (m_chunk.m_validDDS) {
            auto const  reserved1 = GetReserved1();
            return ByteArrayToWCstr(wcstr, sizeInWords, reserved1.data(), reserved1.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    size_t Loader::GetReserved1AsHexDump(wchar_t* wcstr, size_t sizeInWords)const {
        if (m_chunk.m_validDDS) {
            auto reserved1 = GetReserved1();
            return ConvertByteArrayToWCstrHex(wcstr, sizeInWords, reserved1.data(), reserved1.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    size_t Loader::GetReserved1AsAsciiDump(wchar_t* wcstr, size_t sizeInWords)const {
        if (m_chunk.m_validDDS) {
            auto reserved1 = GetReserved1();
            return ConvertByteArrayToWCstrAscii(wcstr, sizeInWords, reserved1.data(), reserved1.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    DWORD Loader::GetDDPFFlags() const{
        return m_chunk.m_header.dx7.ddspf.dwFlags;
    }

    size_t Loader::GetDDPFFlagsAsWChar(wchar_t* const wcstr, size_t sizeInWords)const {
        if (! m_chunk.m_validDDS) {
            return MakeEmptyWStr(wcstr,sizeInWords);
        }
        return dds_loader_flags::GetDDPFFlagsAsWChar(wcstr,sizeInWords,GetDDPFFlags());
    }

    /////////////////////////////////////////////////////////////////////////////////
    // DX10
    /////////////////////////////////////////////////////////////////////////////////

    DWORD Loader::GetDx10Format()const {
        return m_chunk.m_header.dx10.dwFormat;
    }

    size_t Loader::GetDx10FormatAsDecimal(wchar_t* wcstr, size_t sizeInWords)const {
        if (! m_chunk.m_validDDS) {
            return MakeEmptyWStr(wcstr,sizeInWords);
        }
        return DWordToDecimalWCStr(wcstr,sizeInWords,GetDx10Format());
    }

    size_t Loader::GetDx10FormatAsWChar(wchar_t* wcstr, size_t sizeInWords)const {
        if (! m_chunk.m_validDDS) {
            return MakeEmptyWStr(wcstr,sizeInWords);
        }
        return dds_loader_flags::GetDx10FormatAsWChar(wcstr, sizeInWords,GetDx10Format());
    }

    DWORD Loader::GetDx10Dimension()const {
        return m_chunk.m_header.dx10.dwDimension;
    }
    DWORD Loader::GetDx10MiscFlag()const {
        return m_chunk.m_header.dx10.dwMiscFlag;
    }
    DWORD Loader::GetDx10ArraySize()const {
        return m_chunk.m_header.dx10.dwArraySize;
    }
    DWORD Loader::GetDx10MiscFlag2()const {
        return m_chunk.m_header.dx10.dwMiscFlag2;
    }

}; /*namespace dds_loadert*/
