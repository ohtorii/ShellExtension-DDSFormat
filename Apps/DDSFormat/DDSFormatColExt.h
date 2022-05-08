#pragma once

#ifndef	DDSFORMATCOLEXT_H
#define	DDSFORMATCOLEXT_H

#include "Resource/resource.h"
#include "GeneratedCode/GeneratedCode.h"

namespace dds_format {
    class ATL_NO_VTABLE CDDSFormatColExt :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<CDDSFormatColExt, &CLSID_DDSFormatColExt>,
        public IColumnProvider
    {
    public:
        CDDSFormatColExt() { }

        BEGIN_COM_MAP(CDDSFormatColExt)
            COM_INTERFACE_ENTRY_IID(IID_IColumnProvider, IColumnProvider)
        END_COM_MAP()

        DECLARE_REGISTRY_RESOURCEID(IDR_DDSFORMATCOLEXT)

    public:
        STDMETHODIMP Initialize(LPCSHCOLUMNINIT psci);
        STDMETHODIMP GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO* psci);
        STDMETHODIMP GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT* pvarData);
    };
};

#endif /* DDSFORMATCOLEXT_H */
