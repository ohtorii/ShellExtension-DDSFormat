#pragma once

#ifndef	DDSFORMATCOLEXT_H
#define	DDSFORMATCOLEXT_H

#include "Resource/resource.h"
#include "GeneratedCode/GeneratedCode.h"
#include "DDSFormatColExtCache.h"

namespace dds_loader {
    class Loader;
};


namespace dds_format {
    class ATL_NO_VTABLE CDDSFormatColExt :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<CDDSFormatColExt, &CLSID_DDSFormatColExt>,
        public IColumnProvider
    {
    public:
        CDDSFormatColExt();

        BEGIN_COM_MAP(CDDSFormatColExt)
            COM_INTERFACE_ENTRY_IID(IID_IColumnProvider, IColumnProvider)
        END_COM_MAP()
        DECLARE_REGISTRY_RESOURCEID(IDR_DDSFORMATCOLEXT)

        STDMETHODIMP Initialize(LPCSHCOLUMNINIT psci);
        STDMETHODIMP GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO* psci);
        STDMETHODIMP GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT* pvarData);
    private:
        STDMETHODIMP CreateItemData(dds_loader::Loader &ddsLoader, LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT* pvarData);
        FileCache       m_fileCache;
        ColumnCache     m_columnCache;
    };
};

#endif /* DDSFORMATCOLEXT_H */
