#include "DDSFormatFlags.h"
#include "DDSLoader.h"
#include "DDSFormatUtils.h"

#include <string>
#include <dxgiformat.h>



using namespace dds_loader_utils;

namespace {
    struct FlagTableItem {
        const DWORD flag;
        const wchar_t* const str;
        const size_t strlen;
    };

    consteval size_t PixelFormatMinimumBufferCount(const FlagTableItem* const tables, size_t const tableCount) {
        //1 == '\0'のぶん
        size_t result = 1;
        for (size_t i = 0; i < tableCount; ++i) {
            //1=='|'のぶん
            result += tables[i].strlen + 1;
        }
        return result;
    }

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

        constexpr DWORD DDSCAPS2_CUBEMAP = 0x00000200;
        constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
        constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
        constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
        constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
        constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
        constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
        constexpr DWORD DDSCAPS2_VOLUME = 0x00400000;
    };


    size_t MakeFlagsWStr(wchar_t* const wcstr, size_t sizeInWords, const DWORD flags, const FlagTableItem * const tables, const size_t tablesCount)
    {
        wchar_t* dst = wcstr;
        bool     writeVerticalBar = false;
        for (size_t i = 0; i < tablesCount; ++i) {
            const auto& item = tables[i];
            if (!(item.flag & flags)) {
                continue;
            }

            if (writeVerticalBar) [[likely]] {
                *dst = L'|';
                --sizeInWords;
                ++dst;
            }
            else {
                writeVerticalBar = true;
            }
            if (wcscpy_s(dst, sizeInWords, item.str) != 0) {
                //error
                return MakeEmptyWStr(wcstr, sizeInWords);
            }

            dst += item.strlen;
            if ((item.strlen) <= sizeInWords) {
                sizeInWords -= item.strlen;
            }
            else {
                //書き込み先バッファが足りない
                return MakeEmptyWStr(wcstr, sizeInWords);
            }
        }
        //+1 == '\0'のぶん
        return std::distance(wcstr, dst) + 1;
    }
    size_t MakeValueWStr(wchar_t* const wcstr, size_t sizeInWords, const DWORD value, const FlagTableItem* const tables, const size_t tablesCount) {
        const auto* first   = tables;
        const auto* end = tables + tablesCount;
        for (; first != end; ++first) {
            if (value != first->flag) {
                continue;
            }
            if (wcscpy_s(wcstr, sizeInWords, first->str) != 0) {
                //error
                return MakeEmptyWStr(wcstr, sizeInWords);
            }
            //+! == '\0'のぶん
            return first->strlen + 1;
        }
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //  DDPF_*
    ///////////////////////////////////////////////////////////////////////////////
    #define MAKE_DDPF_ITEM(flag) dds_flags::DDPF_##flag, L""#flag, std::char_traits<wchar_t>::length(L""#flag)
    constexpr FlagTableItem sg_ddpf_tables[] = {
        {MAKE_DDPF_ITEM(ALPHAPIXELS)},
        {MAKE_DDPF_ITEM(ALPHA)},
        {MAKE_DDPF_ITEM(FOURCC)},
        {MAKE_DDPF_ITEM(PALETTEINDEXED4)},
        {MAKE_DDPF_ITEM(PALETTEINDEXED8)},
        {MAKE_DDPF_ITEM(RGB)},
        {MAKE_DDPF_ITEM(LUMINANCE)},
        {MAKE_DDPF_ITEM(BUMPDUDV)},
    };
    static_assert(PixelFormatMinimumBufferCount(sg_ddpf_tables, _countof(sg_ddpf_tables)) == static_cast<size_t>(dds_loader::Loader::MinimumBufferCount::PixelFormat));


    ///////////////////////////////////////////////////////////////////////////////
    //  DDSCAPS_*
    ///////////////////////////////////////////////////////////////////////////////
    #define MAKE_DDSCAPS_ITEM(flag) DDSCAPS_##flag, L""#flag, std::char_traits<wchar_t>::length(L""#flag)

    constexpr DWORD DDSCAPS_ALPHA	=0x00000002;
    constexpr DWORD DDSCAPS_COMPLEX	=0x00000008;
    constexpr DWORD DDSCAPS_TEXTURE	=0x00001000;
    constexpr DWORD DDSCAPS_MIPMAP	=0x00400000;

    constexpr FlagTableItem sg_caps_tables[] = {
        {MAKE_DDSCAPS_ITEM(ALPHA)},
        {MAKE_DDSCAPS_ITEM(COMPLEX)},
        {MAKE_DDSCAPS_ITEM(TEXTURE)},
        {MAKE_DDSCAPS_ITEM(MIPMAP)},
    };

    static_assert(PixelFormatMinimumBufferCount(sg_caps_tables, _countof(sg_caps_tables)) == static_cast<size_t>(dds_loader::Loader::MinimumBufferCount::Caps));

    ///////////////////////////////////////////////////////////////////////////////
    //  DDSCAPS2_*
    ///////////////////////////////////////////////////////////////////////////////
    constexpr DWORD DDSCAPS2_CUBEMAP	=0x00000200;
    constexpr DWORD DDSCAPS2_POSITIVE_X	=0x00000400;
    constexpr DWORD DDSCAPS2_NEGATIVE_X	=0x00000800;
    constexpr DWORD DDSCAPS2_POSITIVE_Y	=0x00001000;
    constexpr DWORD DDSCAPS2_NEGATIVE_Y	=0x00002000;
    constexpr DWORD DDSCAPS2_POSITIVE_Z	=0x00004000;
    constexpr DWORD DDSCAPS2_NEGATIVE_Z	=0x00008000;
    constexpr DWORD DDSCAPS2_VOLUME	    =0x00400000;

    #define MAKE_DDSCAPS2_ITEM(flag) DDSCAPS2_##flag, L""#flag, std::char_traits<wchar_t>::length(L""#flag)
    constexpr FlagTableItem sg_caps2_tables[] = {
        {MAKE_DDSCAPS2_ITEM(CUBEMAP)},
        {MAKE_DDSCAPS2_ITEM(POSITIVE_X)},
        {MAKE_DDSCAPS2_ITEM(NEGATIVE_X)},
        {MAKE_DDSCAPS2_ITEM(POSITIVE_Y)},
        {MAKE_DDSCAPS2_ITEM(NEGATIVE_Y)},
        {MAKE_DDSCAPS2_ITEM(POSITIVE_Z)},
        {MAKE_DDSCAPS2_ITEM(NEGATIVE_Z)},
        {MAKE_DDSCAPS2_ITEM(VOLUME)},
    };

    static_assert(PixelFormatMinimumBufferCount(sg_caps2_tables, _countof(sg_caps2_tables)) == static_cast<size_t>(dds_loader::Loader::MinimumBufferCount::Caps2));


    ///////////////////////////////////////////////////////////////////////////////
    //  DXGI_FORMAT_*
    ///////////////////////////////////////////////////////////////////////////////
    #define MAKE_DXGIFORMAT_ITEM(value) DXGI_FORMAT_##value, L""#value, std::char_traits<wchar_t>::length(L""#value)
    constexpr FlagTableItem sg_dxgi_tables[] = {
        {MAKE_DXGIFORMAT_ITEM(UNKNOWN	                              )},
        {MAKE_DXGIFORMAT_ITEM(R32G32B32A32_TYPELESS                   )},
        {MAKE_DXGIFORMAT_ITEM(R32G32B32A32_FLOAT                      )},
        {MAKE_DXGIFORMAT_ITEM(R32G32B32A32_UINT                       )},
        {MAKE_DXGIFORMAT_ITEM(R32G32B32A32_SINT                       )},
        {MAKE_DXGIFORMAT_ITEM(R32G32B32_TYPELESS                      )},
        {MAKE_DXGIFORMAT_ITEM(R32G32B32_FLOAT                         )},
        {MAKE_DXGIFORMAT_ITEM(R32G32B32_UINT                          )},
        {MAKE_DXGIFORMAT_ITEM(R32G32B32_SINT                          )},
        {MAKE_DXGIFORMAT_ITEM(R16G16B16A16_TYPELESS                   )},
        {MAKE_DXGIFORMAT_ITEM(R16G16B16A16_FLOAT                      )},
        {MAKE_DXGIFORMAT_ITEM(R16G16B16A16_UNORM                      )},
        {MAKE_DXGIFORMAT_ITEM(R16G16B16A16_UINT                       )},
        {MAKE_DXGIFORMAT_ITEM(R16G16B16A16_SNORM                      )},
        {MAKE_DXGIFORMAT_ITEM(R16G16B16A16_SINT                       )},
        {MAKE_DXGIFORMAT_ITEM(R32G32_TYPELESS                         )},
        {MAKE_DXGIFORMAT_ITEM(R32G32_FLOAT                            )},
        {MAKE_DXGIFORMAT_ITEM(R32G32_UINT                             )},
        {MAKE_DXGIFORMAT_ITEM(R32G32_SINT                             )},
        {MAKE_DXGIFORMAT_ITEM(R32G8X24_TYPELESS                       )},
        {MAKE_DXGIFORMAT_ITEM(D32_FLOAT_S8X24_UINT                    )},
        {MAKE_DXGIFORMAT_ITEM(R32_FLOAT_X8X24_TYPELESS                )},
        {MAKE_DXGIFORMAT_ITEM(X32_TYPELESS_G8X24_UINT                 )},
        {MAKE_DXGIFORMAT_ITEM(R10G10B10A2_TYPELESS                    )},
        {MAKE_DXGIFORMAT_ITEM(R10G10B10A2_UNORM                       )},
        {MAKE_DXGIFORMAT_ITEM(R10G10B10A2_UINT                        )},
        {MAKE_DXGIFORMAT_ITEM(R11G11B10_FLOAT                         )},
        {MAKE_DXGIFORMAT_ITEM(R8G8B8A8_TYPELESS                       )},
        {MAKE_DXGIFORMAT_ITEM(R8G8B8A8_UNORM                          )},
        {MAKE_DXGIFORMAT_ITEM(R8G8B8A8_UNORM_SRGB                     )},
        {MAKE_DXGIFORMAT_ITEM(R8G8B8A8_UINT                           )},
        {MAKE_DXGIFORMAT_ITEM(R8G8B8A8_SNORM                          )},
        {MAKE_DXGIFORMAT_ITEM(R8G8B8A8_SINT                           )},
        {MAKE_DXGIFORMAT_ITEM(R16G16_TYPELESS                         )},
        {MAKE_DXGIFORMAT_ITEM(R16G16_FLOAT                            )},
        {MAKE_DXGIFORMAT_ITEM(R16G16_UNORM                            )},
        {MAKE_DXGIFORMAT_ITEM(R16G16_UINT                             )},
        {MAKE_DXGIFORMAT_ITEM(R16G16_SNORM                            )},
        {MAKE_DXGIFORMAT_ITEM(R16G16_SINT                             )},
        {MAKE_DXGIFORMAT_ITEM(R32_TYPELESS                            )},
        {MAKE_DXGIFORMAT_ITEM(D32_FLOAT                               )},
        {MAKE_DXGIFORMAT_ITEM(R32_FLOAT                               )},
        {MAKE_DXGIFORMAT_ITEM(R32_UINT                                )},
        {MAKE_DXGIFORMAT_ITEM(R32_SINT                                )},
        {MAKE_DXGIFORMAT_ITEM(R24G8_TYPELESS                          )},
        {MAKE_DXGIFORMAT_ITEM(D24_UNORM_S8_UINT                       )},
        {MAKE_DXGIFORMAT_ITEM(R24_UNORM_X8_TYPELESS                   )},
        {MAKE_DXGIFORMAT_ITEM(X24_TYPELESS_G8_UINT                    )},
        {MAKE_DXGIFORMAT_ITEM(R8G8_TYPELESS                           )},
        {MAKE_DXGIFORMAT_ITEM(R8G8_UNORM                              )},
        {MAKE_DXGIFORMAT_ITEM(R8G8_UINT                               )},
        {MAKE_DXGIFORMAT_ITEM(R8G8_SNORM                              )},
        {MAKE_DXGIFORMAT_ITEM(R8G8_SINT                               )},
        {MAKE_DXGIFORMAT_ITEM(R16_TYPELESS                            )},
        {MAKE_DXGIFORMAT_ITEM(R16_FLOAT                               )},
        {MAKE_DXGIFORMAT_ITEM(D16_UNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(R16_UNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(R16_UINT                                )},
        {MAKE_DXGIFORMAT_ITEM(R16_SNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(R16_SINT                                )},
        {MAKE_DXGIFORMAT_ITEM(R8_TYPELESS                             )},
        {MAKE_DXGIFORMAT_ITEM(R8_UNORM                                )},
        {MAKE_DXGIFORMAT_ITEM(R8_UINT                                 )},
        {MAKE_DXGIFORMAT_ITEM(R8_SNORM                                )},
        {MAKE_DXGIFORMAT_ITEM(R8_SINT                                 )},
        {MAKE_DXGIFORMAT_ITEM(A8_UNORM                                )},
        {MAKE_DXGIFORMAT_ITEM(R1_UNORM                                )},
        {MAKE_DXGIFORMAT_ITEM(R9G9B9E5_SHAREDEXP                      )},
        {MAKE_DXGIFORMAT_ITEM(R8G8_B8G8_UNORM                         )},
        {MAKE_DXGIFORMAT_ITEM(G8R8_G8B8_UNORM                         )},
        {MAKE_DXGIFORMAT_ITEM(BC1_TYPELESS                            )},
        {MAKE_DXGIFORMAT_ITEM(BC1_UNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(BC1_UNORM_SRGB                          )},
        {MAKE_DXGIFORMAT_ITEM(BC2_TYPELESS                            )},
        {MAKE_DXGIFORMAT_ITEM(BC2_UNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(BC2_UNORM_SRGB                          )},
        {MAKE_DXGIFORMAT_ITEM(BC3_TYPELESS                            )},
        {MAKE_DXGIFORMAT_ITEM(BC3_UNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(BC3_UNORM_SRGB                          )},
        {MAKE_DXGIFORMAT_ITEM(BC4_TYPELESS                            )},
        {MAKE_DXGIFORMAT_ITEM(BC4_UNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(BC4_SNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(BC5_TYPELESS                            )},
        {MAKE_DXGIFORMAT_ITEM(BC5_UNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(BC5_SNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(B5G6R5_UNORM                            )},
        {MAKE_DXGIFORMAT_ITEM(B5G5R5A1_UNORM                          )},
        {MAKE_DXGIFORMAT_ITEM(B8G8R8A8_UNORM                          )},
        {MAKE_DXGIFORMAT_ITEM(B8G8R8X8_UNORM                          )},
        {MAKE_DXGIFORMAT_ITEM(R10G10B10_XR_BIAS_A2_UNORM              )},
        {MAKE_DXGIFORMAT_ITEM(B8G8R8A8_TYPELESS                       )},
        {MAKE_DXGIFORMAT_ITEM(B8G8R8A8_UNORM_SRGB                     )},
        {MAKE_DXGIFORMAT_ITEM(B8G8R8X8_TYPELESS                       )},
        {MAKE_DXGIFORMAT_ITEM(B8G8R8X8_UNORM_SRGB                     )},
        {MAKE_DXGIFORMAT_ITEM(BC6H_TYPELESS                           )},
        {MAKE_DXGIFORMAT_ITEM(BC6H_UF16                               )},
        {MAKE_DXGIFORMAT_ITEM(BC6H_SF16                               )},
        {MAKE_DXGIFORMAT_ITEM(BC7_TYPELESS                            )},
        {MAKE_DXGIFORMAT_ITEM(BC7_UNORM                               )},
        {MAKE_DXGIFORMAT_ITEM(BC7_UNORM_SRGB                          )},
        {MAKE_DXGIFORMAT_ITEM(AYUV                                    )},
        {MAKE_DXGIFORMAT_ITEM(Y410                                    )},
        {MAKE_DXGIFORMAT_ITEM(Y416                                    )},
        {MAKE_DXGIFORMAT_ITEM(NV12                                    )},
        {MAKE_DXGIFORMAT_ITEM(P010                                    )},
        {MAKE_DXGIFORMAT_ITEM(P016                                    )},
        {MAKE_DXGIFORMAT_ITEM(420_OPAQUE                              )},
        {MAKE_DXGIFORMAT_ITEM(YUY2                                    )},
        {MAKE_DXGIFORMAT_ITEM(Y210                                    )},
        {MAKE_DXGIFORMAT_ITEM(Y216                                    )},
        {MAKE_DXGIFORMAT_ITEM(NV11                                    )},
        {MAKE_DXGIFORMAT_ITEM(AI44                                    )},
        {MAKE_DXGIFORMAT_ITEM(IA44                                    )},
        {MAKE_DXGIFORMAT_ITEM(P8                                      )},
        {MAKE_DXGIFORMAT_ITEM(A8P8                                    )},
        {MAKE_DXGIFORMAT_ITEM(B4G4R4A4_UNORM                          )},
        {MAKE_DXGIFORMAT_ITEM(P208                                    )},
        {MAKE_DXGIFORMAT_ITEM(V208                                    )},
        {MAKE_DXGIFORMAT_ITEM(V408                                    )},
        {MAKE_DXGIFORMAT_ITEM(SAMPLER_FEEDBACK_MIN_MIP_OPAQUE         )},
        {MAKE_DXGIFORMAT_ITEM(SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE )},
    };

};

namespace dds_loader_flags {
    size_t GetDDPFFlagsAsWChar(wchar_t* const wcstr, size_t sizeInWords, DWORD flags) {
        return MakeFlagsWStr(wcstr, sizeInWords, flags, sg_ddpf_tables, _countof(sg_ddpf_tables));
    }

    size_t GetCapsAsWChar(wchar_t* wcstr, size_t sizeInWords, DWORD caps) {
        return MakeFlagsWStr(wcstr, sizeInWords, caps, sg_caps_tables, _countof(sg_caps_tables));
    }

    size_t GetCaps2AsWChar(wchar_t* wcstr, size_t sizeInWords, DWORD caps2) {
        return MakeFlagsWStr(wcstr, sizeInWords, caps2, sg_caps2_tables, _countof(sg_caps2_tables));
    }

    size_t GetDx10FormatAsWChar(wchar_t* wcstr, size_t sizeInWords,DWORD format) {
        return MakeValueWStr(wcstr, sizeInWords, format, sg_dxgi_tables, _countof(sg_dxgi_tables));
    }
};

