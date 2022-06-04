#pragma once
#include<windows.h>

namespace dds_loader {

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
        DWORD    dwFormat;      //DXGI_FORMAT (dxgiformat.h éQè∆)
        DWORD    dwDimension;
        DWORD    dwMiscFlag;
        DWORD    dwArraySize;
        DWORD    dwMiscFlag2;
    };
    struct DDS_HEADER {
        DDS_HEADER_DX7      dx7;
        DDS_HEADER_DX10     dx10;
    };
    static_assert(sizeof(DDS_HEADER_DX7)==128);
    static_assert(sizeof(DDS_HEADER)    ==(128+20));

    struct Chunk {
        DDS_HEADER  m_header;
        bool        m_validDDS;
        bool        m_isDX10;
    };

}; /*namespace dds_loader*/