#include"DDSFormatUtils.h"
#include <cstdio>
#include <cassert>
#include <string>

namespace {
    //(Memo)  0x44 -> 'D'
    constexpr size_t sg_asciiLength = 1;;
    //(Memo)  0xFF -> "FF "
    constexpr size_t sg_hexLength = 3;
}

namespace dds_loader_utils {

    size_t MakeEmptyWStr(wchar_t* wcstr, size_t sizeInWords) {
        if (sizeInWords == 0) {
            return 0;
        }
        wcstr[0] = L'\0';
        return 1;
    }
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

    size_t DWordToHexWCStr(wchar_t* dst, size_t sizeInWords,DWORD value) {
        auto writeLen = _snwprintf_s(dst, sizeInWords, sizeInWords, L"0x%08x", value);
        if(writeLen==10){
            return writeLen + 1;
        }
        return MakeEmptyWStr(dst,sizeInWords);
    }
    size_t DWordToDecimalWCStr(wchar_t* dst, size_t sizeInWords,DWORD value) {
        auto writeLen = _snwprintf_s(dst, sizeInWords, sizeInWords, L"%u", value);
        if(writeLen==10){
            return writeLen + 1;
        }
        return MakeEmptyWStr(dst,sizeInWords);
    }

    bool IsPrintableChar(int c) {
        if ((c < ' ') || ('~' < c)) {
            return false;
        }
        return true;
    }

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

};

