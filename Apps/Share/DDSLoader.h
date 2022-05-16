#pragma once
#ifndef	DDSFORMAT_DDSLOADER_H
#define	DDSFORMAT_DDSLOADER_H

#include <windows.h>
#include <array>
#include<string>

namespace dds_loader {
    class Loader {
    private:
        struct DDS_PIXELFORMAT_DX7 {
            DWORD    dwSize;
            DWORD    dwFlags;
            DWORD    dwFourCC;
            DWORD    dwRGBBitCount;
            DWORD    dwRBitMask;
            DWORD    dwGBitMask;
            DWORD    dwBBitMask;
            DWORD    dwABitMask;
        };

        struct DDS_HEADER_DX7 {
            DWORD    dwSignature;
            DWORD    dwSize;
            DWORD    dwFlags;
            DWORD    dwHeight;
            DWORD    dwWidth;
            DWORD    dwPitchOrLinearSize;
            DWORD    dwDepth;
            DWORD    dwMipMapCount;
            DWORD    dwReserved1[11];
            DDS_PIXELFORMAT_DX7 ddspf;
            DWORD    dwCaps;
            DWORD    dwCaps2;
            DWORD    dwCaps3;
            DWORD    dwCaps4;
            DWORD    dwReserved2;
        };
        struct DDS_HEADER_DX10 {
            DWORD    dwFormat;
            DWORD    dwDimension;
            DWORD    dwMiscFlag;
            DWORD    dwArraySize;
            DWORD    dwMiscFlag2;
        };
        struct alignas(16) DDS_HEADER {
            DDS_HEADER_DX7      dx7;
            DDS_HEADER_DX10     dx10;
        };

        /*enum {
            DDS10_DIMENSION_1D = 2,
            DDS10_DIMENSION_2D = 3,
            DDS10_DIMENSION_3D = 4,
        };*/

        enum class MemberSize : size_t{
            FourCCSize      = sizeof(Loader::DDS_PIXELFORMAT_DX7::dwFourCC),
            CapsSize        = sizeof(Loader::DDS_HEADER_DX7::dwCaps),
            Caps2Size       = sizeof(Loader::DDS_HEADER_DX7::dwCaps2),
            Reserved1Size   = sizeof(DDS_HEADER_DX7::dwReserved1),
        };
    public:
        Loader(const Loader&) = delete;
        Loader& operator=(const Loader&) = delete;

        Loader();
        Loader(const wchar_t* fileName);
        bool Load(const wchar_t* fileName);

        enum class MinimumBufferCount :size_t {
            //+1 == '\0'のぶん
            FourCCAsciiDump = static_cast<size_t>(MemberSize::FourCCSize) + 1,

            //1Byteは最大3文字へ変換される
            //  0xFFA1 -> L"FF A1\0"
            //  0xFF   -> l"FF\0"
            FourCCHexDump = static_cast<size_t>(MemberSize::FourCCSize) * 3 + 1,

            //+1 == '\0'のぶん
            Caps    = std::char_traits<wchar_t>::length(L"ALPHA|COMPLEX|TEXTURE|MIPMAP|") + 1,

            //+1 == '\0'のぶん
            Caps2   = std::char_traits<wchar_t>::length(L"CUBEMAP|POSITIVE_X|NEGATIVE_X|POSITIVE_Y|NEGATIVE_Y|POSITIVE_Z|NEGATIVE_Z|VOLUME|")+1,

            //+1 == '\0'のぶん
            Reserved1 = static_cast<size_t>(MemberSize::Reserved1Size) + 1,

            //1Byteは最大3文字へ変換される
            //  0xFFA1 -> L"FF A1\0"
            //  0xFF   -> l"FF\0"
            Reserved1HexDump = static_cast<size_t>(MemberSize::Reserved1Size) * 3 + 1,
            //+1 == '\0'のぶん
            Reserved1AsciiDump = static_cast<size_t>(MemberSize::Reserved1Size) + 1,

            PixelFormat = std::char_traits<wchar_t>::length(L"ALPHAPIXELS|ALPHA|FOURCC|PALETTEINDEXED4|PALETTEINDEXED8|RGB|LUMINANCE|BUMPDUDV|") + 1,
        };

        /// <summary>
        ///  FourCCをバイト配列で得る
        /// (Ex. 1) {'D','X','1','0'}
        /// (Ex. 2) {0x00, 0x00, 0x00, 0x24}  //<--A16B16G16R16
        /// </summary>
        /// <returns>フォーマットのバイト配列</returns>
        std::array<BYTE, static_cast<size_t>(MemberSize::FourCCSize)> GetFourCC()const;

        /// <summary>
        /// FourCCをワイド文字列で受け取る
        /// (Ex. 1) L"DX10"
        /// (Ex. 2) L"...."  //<--0x00000024(A16B16G16R16)は全てさせ移御文字（表示できない）なので L"...." となる
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::AsciiDumpFourCC)</param>
        /// <returns>変換された文字数</returns>
        size_t GetFourCCAsAsciiDump(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// FourCCをフォーマットで取得する
        /// (Ex. 1) {'D','X','1','0'}
        /// (Ex. 2) {'A','1','6','B','1','6','G','1','6','R','1','6'}
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::FourCCHexDump)</param>
        /// <returns></returns>
        size_t GetFourCCAsHexDump(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// DDS_HEADER_DX7::dwMipMapCountを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetMipMapCount()const;

        /// <summary>
        /// DDS_HEADER_DX7::dwDepthを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetDepth()const;

        /// <summary>
        /// DDS_PIXELFORMAT_DX7::dwRGBBitCountを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetRGBBitCount()const;

        /// <summary>
        /// DDS_HEADER_DX7::dwCapsを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetCaps()const;

        /// <summary>
        /// DDS_HEADER_DX7::dwCapsを文字列で取得する
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::Caps)</param>
        /// <returns>変換された文字数</returns>
        size_t GetCapsAsWChar(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// DDS_HEADER_DX7::dwCaps2を取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetCaps2()const;

        /// <summary>
        /// DDS_HEADER_DX7::dwCaps2を文字列で取得する
        /// </summary>
        /// /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::Caps)</param>
        /// <returns>変換された文字数</returns>
        size_t GetCaps2AsWChar(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// dwReserved1をバイト配列で得る
        /// </summary>
        /// <returns></returns>
        std::array<BYTE, static_cast<size_t>(MemberSize::Reserved1Size)> GetReserved1()const;

        /// <summary>
        /// dwReserved1をワイド文字列で得る
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数（最低でもMinimumBufferCount::Reserved1）</param>
        /// <returns>書き込んだ文字数</returns>
        size_t GetReserved1AsWChar(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// dwReserved1を16進数でダンプする
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">>書き込み先の文字数（最低でもvMinimumBufferCount::Reserved1HexDump）</param>
        /// <returns>書き込んだ字数</returns>
        size_t GetReserved1AsHexDump(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// dwReserved1をASCII文字でダンプする
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数（最低でもvMinimumBufferCount::::Reserved1AsciiDump）</param>
        /// <returns>書き込んだ文字数</returns>
        size_t GetReserved1AsAsciiDump(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// DDS_PIXELFORMAT_DX7::dwFlagsを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetDDPFFlags()const;

        /// <summary>
        /// DDS_PIXELFORMAT_DX7::dwFlagsを文字列で取得する
        /// (Ex.) "ALPHA|FOURCC"
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数（最低でもPixelFormat）</param>
        /// <returns>書き込んだ文字数</returns>
        size_t GetDDPFFlagsAsWChar(wchar_t* wcstr, size_t sizeInWords)const;

    private:
        void Initialize();
        DDS_HEADER  m_header;
        bool        m_validDDS;
        bool        m_isDX10;
    };
};

#endif /* DDSFORMAT_DDSLOADER_H */
