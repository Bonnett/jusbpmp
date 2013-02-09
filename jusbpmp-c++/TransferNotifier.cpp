#include "StdAfx.h"
#include "TransferNotifier.h"
#include "jsyncnotifier.h"

CUsbPmpTransferNotifier::CUsbPmpTransferNotifier(void){
	_internalNotif = NULL;
}

CUsbPmpTransferNotifier::~CUsbPmpTransferNotifier(void){
}

HRESULT CUsbPmpTransferNotifier::Begin( DWORD dwEstimatedTicks ){
	if (_internalNotif != NULL){
		((jsyncnotifier*)_internalNotif)->notify_begin(dwEstimatedTicks);
	}
    return S_OK;
}

HRESULT CUsbPmpTransferNotifier::Progress( DWORD dwTranspiredTicks ){
	if (_internalNotif != NULL){
		((jsyncnotifier*)_internalNotif)->notify_current(dwTranspiredTicks);
	}
    return S_OK;
}

HRESULT CUsbPmpTransferNotifier::End(){
	if (_internalNotif != NULL){
		((jsyncnotifier*)_internalNotif)->notify_end();
	}
    return S_OK;
}

//
// custom methods
//


STDMETHODIMP CUsbPmpTransferNotifier::Cancel( void ){
    return S_OK;
}

STDMETHODIMP CUsbPmpTransferNotifier::SetCustomNotifier(unsigned long long notif){
	_internalNotif = reinterpret_cast<jsyncnotifier*>(notif);
	return S_OK;
}



