#include "DDSLoader.h"
#include <cstdio>
#include <cassert>
#include <string>


namespace {
    constexpr DWORD SIGNATURE = 0x20534444;//"DDS " (Little endian)

    namespace dds_flags {
        constexpr DWORD DDPF_ALPHAPIXELS = 0x00000001;
        constexpr DWORD DDPF_ALPHA = 0x00000002;
        constexpr DWORD DDPF_FOURCC = 0x00000004;
        constexpr DWORD DDPF_PALETTEINDEXED4 = 0x00000008;
        constexpr DWORD DDPF_PALETTEINDEXED8 = 0x00000020;
        constexpr DWORD DDPF_RGB = 0x00000040;
        constexpr DWORD DDPF_LUMINANCE = 0x00020000;
        constexpr DWORD DDPF_BUMPDUDV = 0x00080000;

        constexpr DWORD DDSCAPS_ALPHA = 0x00000002;
        constexpr DWORD DDSCAPS_COMPLEX = 0x00000008;
        constexpr DWORD DDSCAPS_TEXTURE = 0x00001000;
        constexpr DWORD DDSCAPS_MIPMAP = 0x00400000;

        constexpr DWORD DDSCAPS2_CUBEMAP	= 0x00000200;
        constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400;
        constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800;
        constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000;
        constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000;
        constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000;
        constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000;
        constexpr DWORD DDSCAPS2_VOLUME	= 0x00400000;
    };

    struct DDPFTableItem {
        const DWORD flag;
        const wchar_t* const str;
        const size_t strlen;
    };

#define MAKE_DDPF_ITEM(flag) dds_flags::DDPF_##flag, L""#flag, std::char_traits<wchar_t>::length(L""#flag)

    constexpr DDPFTableItem sg_ddpf_tables[] = {
        {MAKE_DDPF_ITEM(ALPHAPIXELS)},
        {MAKE_DDPF_ITEM(ALPHA)},
        {MAKE_DDPF_ITEM(FOURCC)},
        {MAKE_DDPF_ITEM(PALETTEINDEXED4)},
        {MAKE_DDPF_ITEM(PALETTEINDEXED8)},
        {MAKE_DDPF_ITEM(RGB)},
        {MAKE_DDPF_ITEM(LUMINANCE)},
        {MAKE_DDPF_ITEM(BUMPDUDV)},
    };
    consteval size_t PixelFormatMinimumBufferCount() {
        //1 == '\0'のぶん
        size_t result = 1;
        for (const auto &item : sg_ddpf_tables) {
            //1=='|'のぶん
            result += item.strlen+1;
        }
        return result;
    }
    static_assert(PixelFormatMinimumBufferCount() == static_cast<size_t>(dds_loader::Loader::MinimumBufferCount::PixelFormat));

    void DWordToByteArray(BYTE dst[4], const DWORD src) {
        dst[0] = static_cast<BYTE>((src & 0x000000ff) >> 0);
        dst[1] = static_cast<BYTE>((src & 0x0000ff00) >> 8);
        dst[2] = static_cast<BYTE>((src & 0x00ff0000) >> 16);
        dst[3] = static_cast<BYTE>((src & 0xff000000) >> 24);
    }

    size_t ByteArrayToWCstr(wchar_t* dst, size_t sizeInWords, const BYTE* src, size_t srcSize) {
        static const errno_t    success = 0;
        size_t                  returnValue = 0;

        if (mbstowcs_s(&returnValue, dst, sizeInWords, reinterpret_cast<const char*>(src), srcSize) == success) {
            return returnValue;
        }
        if (0 < sizeInWords) {
            dst[0] = '\0';
            return 1;
        }
        return 0;
    }

    //(Memo)  0x44 -> 'D'
    constexpr size_t sg_asciiLength = 1;
    //(Memo)  0xFF -> "FF "
    constexpr size_t sg_hexLength = 3;

    /// <summary>
    /// コンバート可能なAscii文字数を計算する(終端の\0は考慮しない)
    /// </summary>
    /// (Ex 1.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 4;
    ///         return     4;
    ///
    /// (Ex 2.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 4+1;
    ///         return     4;
    ///
    /// (Ex 3.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 1;
    ///         return     1;
    /// <param name="dstCount"></param>
    /// <param name="srcCount"></param>
    /// <returns></returns>
    size_t CaclConvertibleAsciiCount(size_t dstCount, size_t srcCount) {
        if ((dstCount < sg_asciiLength) || (srcCount == 0)) {
            return 0;
        }
        //srcからAscii文字列へ変換後の文字数
        //+1 == '\0'のぶん
        const auto dstConvertedMaxLen = srcCount * sg_asciiLength + 1;
        if (dstConvertedMaxLen <= dstCount) {
            return srcCount;
        }
        return dstCount / sg_asciiLength;
    }

    bool IsPrintableChar(int c) {
        if ((c < ' ') || ('~' < c)) {
            return false;
        }
        return true;
    }

    /// <summary>
    /// BYTE配列をASCII文字列へ変換する
    /// (Ex.) src   = [0x44,0x44,0x53,0x20]
    ///       dst   = L"DDS \0"
    ///      return = 5
    /// </summary>
    /// <param name="dst"></param>
    /// <param name="dstCount"></param>
    /// <param name="src"></param>
    /// <param name="srcCount"></param>
    /// <returns></returns>
    size_t ConvertByteArrayToWCstrAscii(wchar_t* const dst, const size_t dstCount, const BYTE* const src, const size_t srcCount) {
        assert(dst != nullptr);
        assert(src != nullptr);

        const size_t convertAsciiLength = CaclConvertibleAsciiCount(dstCount, srcCount);
        for (size_t srcIndex = 0, dstIndex = 0; srcIndex < convertAsciiLength; ++srcIndex, dstIndex += sg_asciiLength) {
            const int srcValue = static_cast<unsigned int>(src[srcIndex]);

            if (IsPrintableChar(srcValue)) {
                //(Memo) +1 == '\0'
                wchar_t dstTemp[sg_asciiLength + 1];

                if (_snwprintf_s(dstTemp, _countof(dstTemp), _countof(dstTemp), L"%C", srcValue) == sg_asciiLength) {
                    //success
                    dst[dstIndex] = dstTemp[0];
                }
                else {
                    //error
                    dst[dstIndex] = L'.';
                }
            }
            else {
                dst[dstIndex] = L'.';
            }
        }

        //バッファを0終端させる
        size_t terminateIndex = 0;
        if ((convertAsciiLength * sg_asciiLength) < dstCount) {
            terminateIndex = convertAsciiLength * sg_asciiLength;
        }
        else {
            terminateIndex = dstCount - 1;
        }
        dst[terminateIndex] = L'\0';

        const size_t writeCount = terminateIndex + 1;
        return writeCount;
    }

    /// <summary>
    /// コンバート可能なAscii文字数を計算する(終端の\0は考慮しない)
    /// (Ex 1.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 12+1;
    ///         return     4;
    ///
    /// (Ex 2.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 12;
    ///         return     4;
    ///
    /// (Ex 3.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 3;
    ///         return     1;
    ///
    /// (Ex 4.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 2;
    ///         return     0;
    /// </summary>
    /// <param name="dstCount"></param>
    /// <param name="srcCount"></param>
    /// <returns></returns>
    size_t ConvertByteArrayToWCstr(size_t dstCount, size_t srcCount) {
        if ((dstCount < sg_hexLength) || (srcCount == 0)) {
            return 0;
        }
        //srcからhex文字列へ変換後の文字数
        //+1 == '\0'のぶん
        const auto dstConvertedMaxLen = srcCount * sg_hexLength + 1;
        if (dstConvertedMaxLen <= dstCount) {
            return srcCount;
        }
        return dstCount / sg_hexLength;
    }
    /// <summary>
    /// BYTE配列を16進数文字列へ変換する
    /// (Ex.) src   = [0x44,0x44,0x53,0x20]
    ///       dst   = L"44 44 53 20\0"
    ///      return = 12
    /// </summary>
    /// <param name="dst"></param>
    /// <param name="dstSize"></param>
    /// <param name="src"></param>
    /// <param name="srcSize"></param>
    /// <returns>書き込んだ文字数</returns>
    size_t ConvertByteArrayToWCstrHex(wchar_t* const dst, const size_t dstCount, const BYTE* const src, const size_t srcCount) {
        assert(dst != nullptr);
        assert(src != nullptr);

        const size_t convertAsciiLength = ConvertByteArrayToWCstr(dstCount, srcCount);
        for (size_t srcIndex = 0, dstIndex = 0; srcIndex < convertAsciiLength; ++srcIndex, dstIndex += sg_hexLength) {
            const int srcValue = static_cast<unsigned int>(src[srcIndex]);
            //(Memo) +1 == '\0'
            wchar_t dstTemp[sg_hexLength + 1];
            if (_snwprintf_s(dstTemp, _countof(dstTemp), _countof(dstTemp), L"%02X ", srcValue) == sg_hexLength) {
                //success
                dst[dstIndex] = dstTemp[0];
                dst[dstIndex + 1] = dstTemp[1];
                dst[dstIndex + 2] = dstTemp[2];
            }
            else {
                //error
                dst[dstIndex] = L' ';
                dst[dstIndex + 1] = L' ';
                dst[dstIndex + 2] = L' ';
            }
        }

        //バッファを0終端させる
        size_t terminateIndex = 0;
        if ((convertAsciiLength * sg_hexLength) < dstCount) {
            terminateIndex = convertAsciiLength * sg_hexLength;
        }
        else {
            terminateIndex = dstCount - 1;
        }
        dst[terminateIndex] = L'\0';

        const size_t writeCount = terminateIndex + 1;
        return writeCount;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="wcstr"></param>
    /// <param name="sizeInWords"></param>
    /// <returns>書き込んだ文字数</returns>
    size_t MakeEmptyWStr(wchar_t* wcstr, size_t sizeInWords) {
        if (sizeInWords == 0) {
            return 0;
        }
        wcstr[0] = L'\0';
        return 1;
    }

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
        memset(&m_header, 0, sizeof(m_header));
        m_validDDS = false;
        m_isDX10 = false;
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
            const auto readCount = fread_s(&m_header, sizeof(m_header), 1, sizeof(m_header), file);
            if (readCount < sizeof(m_header.dx7)) {
                Initialize();
                return false;
            }
            if (readCount < sizeof(m_header.dx10)) {
                m_isDX10 = false;
            }
            else {
                m_isDX10 = true;
            }
            if (m_header.dx7.dwSignature != SIGNATURE) {
                Initialize();
                return false;
            }
        }
        m_validDDS = true;
        return true;
    }

    size_t Loader::GetFourCCAsAsciiDump(wchar_t* wcstr, size_t sizeInWords)const {
        if (m_validDDS) {
            auto const  fourcc = GetFourCC();
            return ConvertByteArrayToWCstrAscii(wcstr, sizeInWords, fourcc.data(), fourcc.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    size_t Loader::GetFourCCAsHexDump(wchar_t* wcstr, size_t sizeInWords)const{
        if (m_validDDS) {
            auto const  fourcc = GetFourCC();
            return ConvertByteArrayToWCstrHex(wcstr, sizeInWords, fourcc.data(), fourcc.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    std::array<BYTE, Loader::FourCCSize> Loader::GetFourCC()const {
        BYTE buf[sizeof(DWORD)];
        DWordToByteArray(buf, m_header.dx7.ddspf.dwFourCC);
        return std::array<BYTE, Loader::FourCCSize>{buf[0], buf[1], buf[2], buf[3]};
    }

    DWORD Loader::GetMipMapCount()const {
        return m_header.dx7.dwMipMapCount;
    }

    std::array<BYTE, Loader::Reserved1Size> Loader::GetReserved1()const {
        std::array<BYTE, Loader::Reserved1Size> result;
        auto it = result.begin();
        for (const DWORD value : m_header.dx7.dwReserved1) {
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
        if (m_validDDS) {
            auto const  reserved1 = GetReserved1();
            return ByteArrayToWCstr(wcstr, sizeInWords, reserved1.data(), reserved1.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    size_t Loader::GetReserved1AsHexDump(wchar_t* wcstr, size_t sizeInWords)const {
        if (m_validDDS) {
            auto reserved1 = GetReserved1();
            return ConvertByteArrayToWCstrHex(wcstr, sizeInWords, reserved1.data(), reserved1.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    size_t Loader::GetReserved1AsAsciiDump(wchar_t* wcstr, size_t sizeInWords)const {
        if (m_validDDS) {
            auto reserved1 = GetReserved1();
            return ConvertByteArrayToWCstrAscii(wcstr, sizeInWords, reserved1.data(), reserved1.size());
        }
        return MakeEmptyWStr(wcstr,sizeInWords);
    }

    size_t Loader::GetDDPFFlags(wchar_t* const wcstr, size_t sizeInWords)const {
        if (! m_validDDS) {
            return MakeEmptyWStr(wcstr,sizeInWords);
        }

        wchar_t*    dst                 = wcstr;
        const auto  flags            = m_header.dx7.ddspf.dwFlags;
        bool        writeVerticalBar = false;
        for (const auto &item : sg_ddpf_tables) {
            if (! (item.flag & flags)) {
                continue;
            }

            if (writeVerticalBar)[[likely]] {
                *dst = L'|';
                --sizeInWords;
                ++dst;
            } else{
                writeVerticalBar = true;
            }
            if (wcscpy_s(dst, sizeInWords, item.str) != 0) {
                //error
                return MakeEmptyWStr(wcstr,sizeInWords);
            }

            dst += item.strlen;
            if ((item.strlen) <= sizeInWords) {
                sizeInWords -= item.strlen;
            }
            else {
                //書き込み先バッファが足りない
                return MakeEmptyWStr(wcstr,sizeInWords);
            }
        }
        //+1 == '\0'のぶん
        return std::distance(wcstr, dst) + 1;
    }


}; /*namespace dds_loadert*/
