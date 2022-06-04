#pragma once
#include<windows.h>

namespace dds_loader_utils {
    /// <summary>
    ///
    /// </summary>
    /// <param name="wcstr"></param>
    /// <param name="sizeInWords"></param>
    /// <returns>書き込んだ文字数</returns>
    size_t MakeEmptyWStr(wchar_t* wcstr, size_t sizeInWords);
    void DWordToByteArray(BYTE dst[4], const DWORD src);
    size_t ByteArrayToWCstr(wchar_t* dst, size_t sizeInWords, const BYTE* src, size_t srcSize);
    size_t DWordToHexWCStr(wchar_t* dst, size_t sizeInWords, DWORD value);
    size_t DWordToDecimalWCStr(wchar_t* dst, size_t sizeInWords, DWORD value);
    bool IsPrintableChar(int c);

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
    size_t CaclConvertibleAsciiCount(size_t dstCount, size_t srcCount);

    /// <summary>
    /// BYTE配列をASCII文字列へ変換する
    /// (e.g.) src   = [0x44,0x44,0x53,0x20]
    ///        dst   = L"DDS \0"
    ///       return = 5
    /// </summary>
    /// <param name="dst"></param>
    /// <param name="dstCount"></param>
    /// <param name="src"></param>
    /// <param name="srcCount"></param>
    /// <returns></returns>
    size_t ConvertByteArrayToWCstrAscii(wchar_t* const dst, const size_t dstCount, const BYTE* const src, const size_t srcCount);


    /// <summary>
    /// コンバート可能なAscii文字数を計算する(終端の\0は考慮しない)
    /// (e.g. 1.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 12+1;
    ///         return     4;
    ///
    /// (e.g. 2.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 12;
    ///         return     4;
    ///
    /// (e.g. 3.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 3;
    ///         return     1;
    ///
    /// (e.g. 4.) srcCount = 4;      //[0x44,0x44,0x53,0x20]
    ///         dstCount = 2;
    ///         return     0;
    /// </summary>
    /// <param name="dstCount"></param>
    /// <param name="srcCount"></param>
    /// <returns></returns>
    size_t ConvertByteArrayToWCstr(size_t dstCount, size_t srcCount);

    /// <summary>
    /// BYTE配列を16進数文字列へ変換する
    /// (e.g.)src   = [0x44,0x44,0x53,0x20]
    ///       dst   = L"44 44 53 20\0"
    ///      return = 12
    /// </summary>
    /// <param name="dst"></param>
    /// <param name="dstSize"></param>
    /// <param name="src"></param>
    /// <param name="srcSize"></param>
    /// <returns>書き込んだ文字数</returns>
    size_t ConvertByteArrayToWCstrHex(wchar_t* const dst, const size_t dstCount, const BYTE* const src, const size_t srcCount);
};

