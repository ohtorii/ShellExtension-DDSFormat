#include "pch.h"
#include "DDSFormatColExt.h"
#include "DDSLoader.h"
#include "Logger.h"
#include <cstring>
#include <cassert>
#include <algorithm>
#include <functional>

///カラムに定数値を返す処理を「有効(1)・無効(0)」する
///定数値を返すことでパフォーマンスの最速値を調査する用途で利用します
#define RETURN_CONSTANT_VALUES_FOR_THE_COLUMNS (0)

namespace {
    enum class Column : DWORD{
        Width=0,
        Height,
        Width2N,
        Height2N,
        Square,
        ForCCAsciiDump,
        ForCCHexDump,
        PixelFormat,
        Depth,
        RGBBitCount,
        RBitMask,
        GBitMask,
        BBitMask,
        ABitMask,
        Caps,
        Caps2,
        MipMapCount,
        Reserved1AsAsciiDump,
        Reserved1AsHexDump,
        Dx10FormatAsChar,
        Dx10FormatAsDecimal,
        Dx10FormatDimensionAsDecimal,
        Dx10FormatMiscFlagAsDecimal,
        Dx10FormatArraySizeAsDecimal,
        Dx10FormatMiscFlag2AsDecimal,

        Number,
    };


    /////////////////////////////////////////////////////////////////////////////////
    // Initialize XXX
    /////////////////////////////////////////////////////////////////////////////////
    HRESULT InitializeAsString(SHCOLUMNINFO* psci, DWORD pid, UINT chars, const wchar_t* title, const wchar_t* description) {
        psci-> scid.fmtid = CLSID_DDSFormatColExt;
        psci-> scid.pid   = pid;
        psci-> vt         = VT_LPSTR;
        psci-> fmt        = LVCFMT_LEFT;
        psci-> csFlags    = SHCOLSTATE_TYPE_STR;
        psci-> cChars     = chars;
        wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), title, _TRUNCATE);
        wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), description, _TRUNCATE);
        return S_OK;
    }

    HRESULT InitializeAsHex(SHCOLUMNINFO* psci, DWORD pid, UINT chars, const wchar_t* title, const wchar_t* description) {
        return InitializeAsString(psci, pid, chars, title, description);
    }

    HRESULT InitializeAsInt(SHCOLUMNINFO* psci, DWORD pid, UINT chars, const wchar_t* title, const wchar_t* description) {
        psci-> scid.fmtid = CLSID_DDSFormatColExt;
        psci-> scid.pid   = pid;
        psci-> vt         = VT_INT;
        psci-> fmt        = LVCFMT_RIGHT;
        psci-> csFlags    = SHCOLSTATE_TYPE_INT;
        psci-> cChars     = chars;
        wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), title, _TRUNCATE);
        wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), description, _TRUNCATE);
        return S_OK;
    }

    HRESULT InitializeAsBool(SHCOLUMNINFO* psci, DWORD pid, UINT chars, const wchar_t* title, const wchar_t* description) {
        psci-> scid.fmtid = CLSID_DDSFormatColExt;
        psci-> scid.pid   = pid;
        psci-> vt         = VT_BOOL;
        psci-> fmt        = LVCFMT_RIGHT;
        psci-> csFlags    = SHCOLSTATE_TYPE_INT;
        psci-> cChars     = chars;
        wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), title, _TRUNCATE);
        wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), description, _TRUNCATE);
        return S_OK;
    }


    /////////////////////////////////////////////////////////////////////////////////
    // Make XXX
    /////////////////////////////////////////////////////////////////////////////////
    HRESULT MakeWStr(dds_loader::Loader*loader, std::function<size_t(dds_loader::Loader*, wchar_t*, size_t)> method, VARIANT* pvarData) {
#if RETURN_CONSTANT_VALUES_FOR_THE_COLUMNS
        CComVariant vData("HOGE");
        return vData.Detach(pvarData);
#else
        std::array<wchar_t,static_cast<size_t>(dds_loader::Loader::MinimumBufferCount::Reserved1HexDump)>      szField;
        szField[0] = '\0';//念のため
        /*auto _ = */ method(loader, szField.data(), szField.size());
        szField.back() = '\0';//念のため
        CComVariant vData(szField.data());
        return vData.Detach(pvarData);
#endif
    }

    HRESULT MakeDWORDAsDecimal(dds_loader::Loader*loader, std::function<DWORD(dds_loader::Loader*)> method, VARIANT* pvarData) {
#if RETURN_CONSTANT_VALUES_FOR_THE_COLUMNS
        CComVariant vData(static_cast<DWORD>(0));
        return vData.Detach(pvarData);
#else
        CComVariant vData(static_cast<DWORD>(method(loader)));
        return vData.Detach(pvarData);
#endif
    }
}; //namespace



namespace dds_format {
    CDDSFormatColExt::CDDSFormatColExt() {
#if 1
        Logger::SetRawWriter(nullptr);
#else
        Logger::SetRawWriter(LoggerOutputDebugString::Write);
#endif
    }

    STDMETHODIMP CDDSFormatColExt::Initialize([[maybe_unused]] LPCSHCOLUMNINIT psci) {
        return S_OK;
    }

    STDMETHODIMP CDDSFormatColExt::GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO* psci)
    {
        switch (dwIndex)
        {
        case static_cast<DWORD>(Column::Width):
            return InitializeAsInt(psci, dwIndex, 5, _T("Width"), _T("Width of DDS"));

        case static_cast<DWORD>(Column::Height):
            return InitializeAsInt(psci, dwIndex, 6, _T("Height"), _T("Height of DDS"));

        case static_cast<DWORD>(Column::Width2N):
            return InitializeAsBool(psci, dwIndex, 6, _T("Width(2^n)"), _T("DDS width is a power of 2?"));

        case static_cast<DWORD>(Column::Height2N):
            return InitializeAsBool(psci, dwIndex, 6, _T("Height(2^n)"), _T("DDS height is a power of 2?"));

        case static_cast<DWORD>(Column::Square):
            return InitializeAsBool(psci, dwIndex, 6, _T("Square"), _T("Is the DDS height-width square?"));

        case static_cast<DWORD>(Column::ForCCAsciiDump):
            return InitializeAsString(psci, dwIndex, 4, _T("FourCC(Ascii)"), _T("FourCC area as ascii dump"));

        case static_cast<DWORD>(Column::ForCCHexDump):
            return InitializeAsString(psci, dwIndex, 4, _T("FourCC(Hex)"), _T("FourCC area as hex dump"));

        case static_cast<DWORD>(Column::PixelFormat):
            return InitializeAsString(psci, dwIndex, 10, _T("PixelFormat"), _T("PixelFormat of DDS"));

        case static_cast<DWORD>(Column::Depth):
            return InitializeAsInt(psci, dwIndex, 2, _T("Depth"), _T("Depth of DDS"));

        case static_cast<DWORD>(Column::RGBBitCount):
            return InitializeAsString(psci, dwIndex, 10, _T("RGBBitCount"), _T("RGBBitCount of DDS"));

        case static_cast<DWORD>(Column::RBitMask):
            return InitializeAsHex(psci, dwIndex, 10, _T("RedBitMask(Hex)"), _T("Red bit mask of DDS"));

        case static_cast<DWORD>(Column::GBitMask):
            return InitializeAsHex(psci, dwIndex, 10, _T("GreenBitMask(Hex)"), _T("Green bit mask of DDS"));

        case static_cast<DWORD>(Column::BBitMask):
            return InitializeAsHex(psci, dwIndex, 2, _T("BlueBitMask(Hex)"), _T("Blue bit mask of DDS"));

        case static_cast<DWORD>(Column::ABitMask):
            return InitializeAsHex(psci, dwIndex, 10, _T("AlphabitMask(Hex)"), _T("Alpha bit mask of DDS"));

        case static_cast<DWORD>(Column::Caps):
            return InitializeAsString(psci, dwIndex, 10, _T("Caps"), _T("Caps of DDS"));

        case static_cast<DWORD>(Column::Caps2):
            return InitializeAsString(psci, dwIndex, 10, _T("Caps2"), _T("Caps2 of DDS"));

        case static_cast<DWORD>(Column::MipMapCount):
            return InitializeAsInt(psci, dwIndex, 2, _T("MipMapCount"), _T("Mipmap count of DDS"));

        case static_cast<DWORD>(Column::Reserved1AsAsciiDump):
            return InitializeAsString(psci, dwIndex, 16, _T("Reserved1(Ascii)"), _T("DDS reserved1 area as Ascii dump"));

        case static_cast<DWORD>(Column::Reserved1AsHexDump):
            return InitializeAsString(psci, dwIndex, 16, _T("Reserved1(Hex)"), _T("DDS reserved1 area as hex dump"));

        case static_cast<DWORD>(Column::Dx10FormatAsChar):
            return InitializeAsString(psci, dwIndex, 16, _T("DX10.Format(char)"), _T("DX10 Formats as char"));

        case static_cast<DWORD>(Column::Dx10FormatAsDecimal):
            return InitializeAsInt(psci, dwIndex, 10, _T("DX10.Format(decimal)"), _T("DX10 Format as decimal"));

        case static_cast<DWORD>(Column::Dx10FormatDimensionAsDecimal):
            return InitializeAsInt(psci, dwIndex, 10, _T("DX10.Dimension"), _T("DX10 dimension as decimal"));

        case static_cast<DWORD>(Column::Dx10FormatMiscFlagAsDecimal):
            return InitializeAsInt(psci, dwIndex, 10, _T("DX10.MiscFlag"), _T("DX10 MiscFlag as decimal"));

        case static_cast<DWORD>(Column::Dx10FormatArraySizeAsDecimal):
            return InitializeAsInt(psci, dwIndex, 10, _T("DX10.ArraySize"), _T("DX10 ArraySize as decimal"));

        case static_cast<DWORD>(Column::Dx10FormatMiscFlag2AsDecimal):
            return InitializeAsInt(psci, dwIndex, 10, _T("DX10.MiscFlag2"), _T("DX10 MiscFlag2 as decimal"));

        default:
            return S_FALSE;
        }
    }

    STDMETHODIMP CDDSFormatColExt::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT* pvarData)
    {
        if (pscid->fmtid != CLSID_DDSFormatColExt)
        {
            return S_FALSE;
        }
        if (static_cast<DWORD>(Column::Number) <= pscid->pid) {
            return S_FALSE;
        }
        if (0 != _wcsicmp(pscd->pwszExt, L".dds")) {
            return S_FALSE;
        }
        if (pscd->dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_OFFLINE)) {
            return S_FALSE;
        }

#if RETURN_CONSTANT_VALUES_FOR_THE_COLUMNS
        dds_loader::Loader ddsLoader;
        return CreateItemData(ddsLoader, pscid, pscd, pvarData);
#else
        //(Memo)以下2種類のデータをキャッシュしている
        // - カラム情報
        // - DDSファイルのヘッダ情報
        if (m_columnCache.Fetch(pscd->wszFile, pscid->pid, pvarData)) {
            return S_OK;
        }else {
            dds_loader::Loader ddsLoader;
            if (m_fileCache.Fetch(pscd->wszFile, ddsLoader.RefChunk()) == false) {
                if (ddsLoader.Load(pscd->wszFile) == false)
                {
                    return S_FALSE;
                }
                m_fileCache.Store(pscd->wszFile, ddsLoader.RefChunk());
            }
            const auto result = CreateItemData(ddsLoader, pscid, pscd, pvarData);
            if (result == S_OK) {
                m_columnCache.Store(pscd->wszFile, pscid->pid, pvarData);
            }
            return result;
        }
#endif
    }

    STDMETHODIMP CDDSFormatColExt::CreateItemData(dds_loader::Loader& ddsLoader, LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT* pvarData)
    {
        (void)pscd;

        switch (pscid->pid)
        {
        case static_cast<DWORD>(Column::Width):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetWidth, pvarData);

        case static_cast<DWORD>(Column::Height):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetHeight, pvarData);

        case static_cast<DWORD>(Column::Width2N):
        case static_cast<DWORD>(Column::Height2N):
        case static_cast<DWORD>(Column::Square):

        case static_cast<DWORD>(Column::ForCCAsciiDump):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetFourCCAsAsciiDump, pvarData);

        case static_cast<DWORD>(Column::ForCCHexDump):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetFourCCAsHexDump, pvarData);

        case static_cast<DWORD>(Column::PixelFormat):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetDDPFFlagsAsWChar, pvarData);

        case static_cast<DWORD>(Column::Depth):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetDepth, pvarData);

        case static_cast<DWORD>(Column::RGBBitCount):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetRGBBitCount, pvarData);

        case static_cast<DWORD>(Column::RBitMask):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetRBitMaskAsHexWChar, pvarData);

        case static_cast<DWORD>(Column::GBitMask):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetGBitMaskAsHexWChar, pvarData);

        case static_cast<DWORD>(Column::BBitMask):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetBBitMaskAsHexWChar, pvarData);

        case static_cast<DWORD>(Column::ABitMask):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetABitMaskAsHexWChar, pvarData);

        case static_cast<DWORD>(Column::Caps):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetCapsAsWChar, pvarData);

        case static_cast<DWORD>(Column::Caps2):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetCaps2AsWChar, pvarData);

        case static_cast<DWORD>(Column::MipMapCount):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetMipMapCount, pvarData);

        case static_cast<DWORD>(Column::Reserved1AsAsciiDump):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetReserved1AsAsciiDump, pvarData);

        case static_cast<DWORD>(Column::Reserved1AsHexDump):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetReserved1AsHexDump, pvarData);

        case static_cast<DWORD>(Column::Dx10FormatAsChar):
            return MakeWStr(&ddsLoader, &dds_loader::Loader::GetDx10FormatAsWChar, pvarData);

        case static_cast<DWORD>(Column::Dx10FormatAsDecimal):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetDx10Format, pvarData);

        case static_cast<DWORD>(Column::Dx10FormatDimensionAsDecimal):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetDx10Dimension, pvarData);

        case static_cast<DWORD>(Column::Dx10FormatMiscFlagAsDecimal):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetDx10MiscFlag, pvarData);

        case static_cast<DWORD>(Column::Dx10FormatArraySizeAsDecimal):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetDx10ArraySize, pvarData);

        case static_cast<DWORD>(Column::Dx10FormatMiscFlag2AsDecimal):
            return MakeDWORDAsDecimal(&ddsLoader, &dds_loader::Loader::GetDx10MiscFlag2, pvarData);

        [[unlikely]]default:
            assert(false);
            return S_FALSE;
        }
    }

}; /*namespace dds_format*/

