#pragma once

#include "resource.h"       // symboles principaux
#include <mswmdm.h>
#include "transfernotifier_h.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Les objets COM monothread ne sont pas correctement pris en charge par les plates-formes Windows CE, notamment les plates-formes Windows Mobile qui ne prennent pas totalement en charge DCOM. Définissez _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA pour forcer ATL à prendre en charge la création d'objets COM monothread et permettre l'utilisation de leurs implémentations. Le modèle de thread de votre fichier rgs a été défini sur 'Libre' car il s'agit du seul modèle de thread pris en charge par les plates-formes Windows CE non-DCOM."
#endif

class ATL_NO_VTABLE CUsbPmpTransferNotifier :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CUsbPmpTransferNotifier, &CLSID_CUsbPmpTransferNotifier>,
	public IDispatchImpl<IJusbPmpTransferNotifier, &IID_IJusbPmpTransferNotifier, &LIBID_JusbPmpComLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IWMDMProgress
{
public:
	CUsbPmpTransferNotifier(void);
	~CUsbPmpTransferNotifier(void);

	DECLARE_REGISTRY_RESOURCEID(IDR_REGISTRY_JUSB1)

BEGIN_COM_MAP(CUsbPmpTransferNotifier)
	COM_INTERFACE_ENTRY(IJusbPmpTransferNotifier)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IWMDMProgress)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	 // IWMDMProgress
    STDMETHOD (Begin)( DWORD dwEstimatedTicks );
    STDMETHOD (Progress)( DWORD dwTranspiredTicks );
    STDMETHOD (End)();

	//custom
	STDMETHOD(Cancel)(void);
	STDMETHOD(SetCustomNotifier)(unsigned long long notif);


private:
	void* _internalNotif;

};

OBJECT_ENTRY_AUTO(__uuidof(CUsbPmpTransferNotifier), CUsbPmpTransferNotifier)