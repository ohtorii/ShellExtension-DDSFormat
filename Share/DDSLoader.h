#pragma once
#ifndef	DDSFORMAT_DDSLOADER_H
#define	DDSFORMAT_DDSLOADER_H

#include <windows.h>
#include <array>

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
        struct DDS_HEADER {
            DDS_HEADER_DX7      dx7;
            DDS_HEADER_DX10     dx10;
        };

        /*enum {
            DDS10_DIMENSION_1D = 2,
            DDS10_DIMENSION_2D = 3,
            DDS10_DIMENSION_3D = 4,
        };*/

        enum {
            FourCCSize = sizeof(Loader::DDS_PIXELFORMAT_DX7::dwFourCC),
            Reserved1Size = sizeof(DDS_HEADER_DX7::dwReserved1),
        };
    public:
        Loader(const Loader&) = delete;
        Loader& operator=(const Loader&) = delete;

        Loader();
        Loader(const wchar_t* fileName);
        bool Load(const wchar_t* fileName);

        enum {
            //+1 == '\0'のぶん
            MinimumFourCCCount = FourCCSize + 1,
            MinimumReserved1Count = Reserved1Size + 1,
            //1Byteは最大3文字へ変換される
            //  0xFFA1 -> L"FF A1\0"
            //  0xFF   -> l"FF\0"
            MinimumReserved1DumpCount = Reserved1Size * 3 + 1
        };

        /// <summary>
        ///  FourCCをバイト配列で得る
        /// </summary>
        /// <returns>フォーマットのバイト配列</returns>
        std::array<BYTE, FourCCSize> GetFourCC()const;

        /// <summary>
        /// FourCCをワイド文字列で受け取る
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumFourCCCount)</param>
        /// <returns>変換された文字数</returns>
        size_t GetFourCCAsWChar(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// dwReserved1をバイト配列で得る
        /// </summary>
        /// <returns></returns>
        std::array<BYTE, Reserved1Size> GetReserved1()const;

        /// <summary>
        /// dwReserved1をワイド文字列で得る
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数（最低でもMinimumReserved1Count）</param>
        /// <returns>書き込んだ文字数</returns>
        size_t GetReserved1AsWChar(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// dwReserved1を16進数でダンプする
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">>書き込み先の文字数（最低でもMinimumReserved1DumpCount）</param>
        /// <returns>書き込んだ字数</returns>
        size_t GetReserved1AsWCharDump(wchar_t* wcstr, size_t sizeInWords)const;

    private:
        void Initialize();
        DDS_HEADER  m_header;
        bool        m_isDX10;;
    };
};

#endif /* DDSFORMAT_DDSLOADER_H */
