#include "DDSLoader.h"
#include <cstdio>
#include <cassert>


namespace {
    constexpr DWORD sg_Signature = 0x20534444;//"DDS " (Little endian)

    void DWordToByteArray(BYTE dst[4], const DWORD src) {
        dst[0]=static_cast<BYTE>((src & 0x000000ff) >> 0);
        dst[1]=static_cast<BYTE>((src & 0x0000ff00) >> 8);
        dst[2]=static_cast<BYTE>((src & 0x00ff0000) >> 16);
        dst[3]=static_cast<BYTE>((src & 0xff000000) >> 24);
    }

    size_t ByteArrayToWCstr(wchar_t*dst, size_t sizeInWords, const BYTE*src, size_t srcSize) {
        static const errno_t    success     = 0;
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

    //(Memo)  0xFF -> "FF "
    const static size_t sg_hexLength      = 3;


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
    size_t CaclConvertibleAsciiCount(size_t dstCount, size_t srcCount) {
        if ((dstCount < sg_hexLength) || (srcCount==0)) {
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
    ///      return = 5
    /// </summary>
    /// <param name="dst"></param>
    /// <param name="dstSize"></param>
    /// <param name="src"></param>
    /// <param name="srcSize"></param>
    /// <returns>書き込んだ文字数</returns>
    size_t ConvertByteArrayToWCstr(wchar_t * const dst, const size_t dstCount, const BYTE* const src, const size_t srcCount) {
        assert(dst != nullptr);
        assert(src != nullptr);

        const size_t convertAsciiLength  = CaclConvertibleAsciiCount(dstCount,srcCount);
        for (size_t srcIndex = 0, dstIndex=0; srcIndex < convertAsciiLength; ++srcIndex, dstIndex+=sg_hexLength) {
            const int srcValue = static_cast<unsigned int>(src[srcIndex]);
            //(Memo) +1 == '\0'
            wchar_t dstTemp[sg_hexLength + 1];
            if (_snwprintf_s(dstTemp, _countof(dstTemp), _countof(dstTemp), L"%02X ", srcValue) == sg_hexLength) {
                //success
                dst[dstIndex]   = dstTemp[0];
                dst[dstIndex+1] = dstTemp[1];
                dst[dstIndex+2] = dstTemp[2];
            }
            else {
                //error
                dst[dstIndex]  = L' ';
                dst[dstIndex+1]= L' ';
                dst[dstIndex+2]= L' ';
            }
        }

        //バッファを0終端させる
        size_t terminateIndex = 0;
        if ((convertAsciiLength*sg_hexLength) < dstCount) {
            terminateIndex = convertAsciiLength*sg_hexLength;
        }
        else {
            terminateIndex = dstCount - 1;
        }
        dst[terminateIndex] = L'\0';

        const size_t writeCount = terminateIndex + 1;
        return writeCount;
    }

    class AutoClose {
    public:
        AutoClose(const AutoClose&)             = delete;
        AutoClose& operator=(const AutoClose&)   = delete;

        AutoClose(FILE *fp) : m_fp(fp){
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
        memset(&m_header,0,sizeof(m_header));
        m_isDX10 = false;
    }

    bool Loader::Load(const wchar_t* fileName) {
        FILE *file=nullptr;
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
            if(readCount < sizeof(m_header.dx7)){
                Initialize();
                return false;
            }
            if (readCount < sizeof(m_header.dx10)) {
                m_isDX10 = false;
            }
            else {
                m_isDX10 = true;
            }
            if (m_header.dx7.dwSignature != sg_Signature) {
                Initialize();
                return false;
            }
        }
        return true;
    }

    size_t Loader::GetFourCCAsWChar(wchar_t* wcstr, size_t sizeInWords)const {
        auto const  format = GetFourCC();
        return ByteArrayToWCstr(wcstr,sizeInWords,format.data(), format.size());
    }

    std::array<BYTE,Loader::FourCCSize> Loader::GetFourCC()const{
        BYTE buf[sizeof(DWORD)];
        DWordToByteArray(buf,m_header.dx7.ddspf.dwFourCC);
        return std::array<BYTE, Loader::FourCCSize>{buf[0],buf[1],buf[2],buf[3]};
    }

    std::array<BYTE, Loader::Reserved1Size> Loader::GetReserved1()const {
        std::array<BYTE, Loader::Reserved1Size> result;
        auto it = result.begin();
        for (const DWORD value : m_header.dx7.dwReserved1) {
            BYTE buf[sizeof(DWORD)];
            DWordToByteArray(buf,value);
            it[0] = buf[0];
            it[1] = buf[1];
            it[2] = buf[2];
            it[3] = buf[3];

            std::advance(it, sizeof(DWORD));
        }
        return result;
    }

    size_t Loader::GetReserved1AsWChar(wchar_t* wcstr, size_t sizeInWords)const {
        auto const  reserved1 = GetReserved1();
        return ByteArrayToWCstr(wcstr,sizeInWords,reserved1.data(), reserved1.size());
    }

    size_t Loader::GetReserved1AsWCharDump(wchar_t* wcstr, size_t sizeInWords)const {
        auto reserved1 = GetReserved1();
        return ConvertByteArrayToWCstr(wcstr, sizeInWords, reserved1.data(), reserved1.size());
    }


}; /*namespace dds_loadert*/
