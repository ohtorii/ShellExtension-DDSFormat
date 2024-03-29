﻿#pragma once
#ifndef	DDSFORMAT_DDSLOADER_H
#define	DDSFORMAT_DDSLOADER_H

#include "internal/DDSStruct.h"
#include <windows.h>
#include <array>
#include <string>

namespace dds_loader {
    class Loader {
    private:

        /*enum {
            DDS10_DIMENSION_1D = 2,
            DDS10_DIMENSION_2D = 3,
            DDS10_DIMENSION_3D = 4,
        };*/
        enum class MemberSize : size_t{
            FourCCSize      = sizeof(DDS_PIXELFORMAT_DX7::dwFourCC),
            CapsSize        = sizeof(DDS_HEADER_DX7::dwCaps),
            Caps2Size       = sizeof(DDS_HEADER_DX7::dwCaps2),
            Reserved1Size   = sizeof(DDS_HEADER_DX7::dwReserved1),
            Dx10Format      = sizeof(DDS_HEADER_DX10::dwFormat),
        };
    public:
        Loader(const Loader&) = delete;
        Loader& operator=(const Loader&) = delete;

        Loader();
        Loader(const wchar_t* fileName);
        bool Load(const wchar_t* fileName);
        Chunk& RefChunk();

        //void Serialize(std::array<BYTE,>&outData)const;
        //void DeSerialize();

        enum class MinimumBufferCount :size_t {
            //+1 == '\0'のぶん
            FourCCAsciiDump = static_cast<size_t>(MemberSize::FourCCSize) + 1,

            //1Byteは最大3文字へ変換される
            //  0xFFA1 -> L"FF A1\0"
            //  0xFF   -> l"FF\0"
            FourCCHexDump = static_cast<size_t>(MemberSize::FourCCSize) * 3 + 1,

            //2 = "0x"
            //8 = DWORD(32bit)の16進数
            //1 = '\0'
            DWordAsHex = 2 + 8 + 1,

            //10 = 4294967296
            //+1 == '\0'のぶん
            DWordAsDecimal= 10 + 1,

            //+1 == '\0'のぶん
            Caps = std::char_traits<wchar_t>::length(L"ALPHA|COMPLEX|TEXTURE|MIPMAP|") + 1,

            //+1 == '\0'のぶん
            Caps2 = std::char_traits<wchar_t>::length(L"CUBEMAP|X+|X-|Y+|Y-|Z+|Z-|VOLUME|") + 1,

            //+1 == '\0'のぶん
            Reserved1 = static_cast<size_t>(MemberSize::Reserved1Size) + 1,

            //1Byteは最大3文字へ変換される
            //  0xFFA1 -> L"FF A1\0"
            //  0xFF   -> l"FF\0"
            Reserved1HexDump = static_cast<size_t>(MemberSize::Reserved1Size) * 3 + 1,

            //+1 == '\0'のぶん
            Reserved1AsciiDump = static_cast<size_t>(MemberSize::Reserved1Size) + 1,

            //+1 == '\0'のぶん
            PixelFormat = std::char_traits<wchar_t>::length(L"ALPHAPIXELS|ALPHA|FOURCC|PALETTEINDEXED4|PALETTEINDEXED8|RGB|LUMINANCE|BUMPDUDV|") + 1,

            //+1 == '\0'のぶん
            Dx10Format = std::char_traits<wchar_t>::length(L"DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE") + 1,
        };

        /// <summary>
        /// 幅を取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetWidth()const;
        /// <summary>
        ///高さを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetHeight()const;
        /// <summary>
        /// 幅が２の冪乗かどうか
        /// </summary>
        /// <returns></returns>
        bool IsWidth2N()const;
        /// <summary>
        /// 高さが２の冪乗かどうか
        /// </summary>
        /// <returns></returns>
        bool IsHeight2N()const;
        /// <summary>
        /// 高さと幅が正方形か
        /// </summary>
        /// <returns></returns>
        bool IsSquare()const;

        /// <summary>
        ///  FourCCをバイト配列で得る
        /// (e.g. 1) {'D','X','1','0'}
        /// (e.g. 2) {0x00, 0x00, 0x00, 0x24}  //<--A16B16G16R16
        /// </summary>
        /// <returns>フォーマットのバイト配列</returns>
        std::array<BYTE, static_cast<size_t>(MemberSize::FourCCSize)> GetFourCC()const;

        /// <summary>
        /// FourCCをワイド文字列で受け取る
        /// (e.g. 1) L"DX10"
        /// (e.g. 2) L"...."  //<--0x00000024(A16B16G16R16)は全てさせ移御文字（表示できない）なので L"...." となる
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::AsciiDumpFourCC)</param>
        /// <returns>変換された文字数</returns>
        size_t GetFourCCAsAsciiDump(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// FourCCをフォーマットで取得する
        /// (e.g. 1) {'D','X','1','0'}
        /// (e.g. 2) {'A','1','6','B','1','6','G','1','6','R','1','6'}
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

        DWORD GetRBitMask()const;
        DWORD GetGBitMask()const;
        DWORD GetBBitMask()const;
        DWORD GetABitMask()const;

        /// <summary>
        ///
        /// </summary>
        /// <param name="wcstr"></param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::DWordAsHex)</param>
        /// <returns></returns>
        size_t GetRBitMaskAsHexWChar(wchar_t* wcstr, size_t sizeInWords)const;
        /// <summary>
        ///
        /// </summary>
        /// <param name="wcstr"></param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::DWordAsHex)</param>
        /// <returns></returns>
        size_t GetGBitMaskAsHexWChar(wchar_t* wcstr, size_t sizeInWords)const;
        /// <summary>
        ///
        /// </summary>
        /// <param name="wcstr"></param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::DWordAsHex)</param>
        /// <returns></returns>
        size_t GetBBitMaskAsHexWChar(wchar_t* wcstr, size_t sizeInWords)const;
        /// <summary>
        ///
        /// </summary>
        /// <param name="wcstr"></param>
        /// <param name="sizeInWords">書き込み先の文字数(最低でもMinimumBufferCount::DWordAsHex)</param>
        /// <returns></returns>
        size_t GetABitMaskAsHexWChar(wchar_t* wcstr, size_t sizeInWords)const;

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
        /// (e.g.) "ALPHA|FOURCC"
        /// </summary>
        /// <param name="wcstr">書き込み先</param>
        /// <param name="sizeInWords">書き込み先の文字数（最低でもPixelFormat）</param>
        /// <returns>書き込んだ文字数</returns>
        size_t GetDDPFFlagsAsWChar(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// DDS_HEADER_DX10::dwFormat を取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetDx10Format()const;

        /// <summary>
        ///
        /// </summary>
        /// <param name="wcstr"></param>
        /// <param name="sizeInWords">書き込み先の文字数（最低でもMinimumBufferCount::DWordAsDecimal）</param>
        /// <returns></returns>
        size_t GetDx10FormatAsDecimal(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        ///
        /// </summary>
        /// <param name="wcstr"></param>
        /// <param name="sizeInWords">書き込み先の文字数（最低でもMinimumBufferCount::Dx10Format）</param>
        /// <returns></returns>
        size_t GetDx10FormatAsWChar(wchar_t* wcstr, size_t sizeInWords)const;

        /// <summary>
        /// DDS_HEADER_DX10::Dimensionを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetDx10Dimension()const;
        /// <summary>
        /// DDS_HEADER_DX10::MiscFlagを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetDx10MiscFlag()const;
        /// <summary>
        /// DDS_HEADER_DX10::ArraySizeを取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetDx10ArraySize()const;
        /// <summary>
        /// DDS_HEADER_DX10::MiscFlag2を取得する
        /// </summary>
        /// <returns></returns>
        DWORD GetDx10MiscFlag2()const;
    private:
        void Initialize();

        Chunk m_chunk;
    };
};

#endif /* DDSFORMAT_DDSLOADER_H */
