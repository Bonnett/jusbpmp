

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Sun Feb 10 00:23:37 2013
 */
/* Compiler settings for .\transfernotifier.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __transfernotifier_h_h__
#define __transfernotifier_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IJusbPmpTransferNotifier_FWD_DEFINED__
#define __IJusbPmpTransferNotifier_FWD_DEFINED__
typedef interface IJusbPmpTransferNotifier IJusbPmpTransferNotifier;
#endif 	/* __IJusbPmpTransferNotifier_FWD_DEFINED__ */


#ifndef __CUsbPmpTransferNotifier_FWD_DEFINED__
#define __CUsbPmpTransferNotifier_FWD_DEFINED__

#ifdef __cplusplus
typedef class CUsbPmpTransferNotifier CUsbPmpTransferNotifier;
#else
typedef struct CUsbPmpTransferNotifier CUsbPmpTransferNotifier;
#endif /* __cplusplus */

#endif 	/* __CUsbPmpTransferNotifier_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_transfernotifier_0000_0000 */
/* [local] */ 




extern RPC_IF_HANDLE __MIDL_itf_transfernotifier_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_transfernotifier_0000_0000_v0_0_s_ifspec;

#ifndef __IJusbPmpTransferNotifier_INTERFACE_DEFINED__
#define __IJusbPmpTransferNotifier_INTERFACE_DEFINED__

/* interface IJusbPmpTransferNotifier */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IJusbPmpTransferNotifier;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E1E7537-EEA9-427b-B31F-9FD32918D045")
    IJusbPmpTransferNotifier : public IDispatch
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCustomNotifier( 
            unsigned long long __MIDL__IJusbPmpTransferNotifier0000) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IJusbPmpTransferNotifierVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IJusbPmpTransferNotifier * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IJusbPmpTransferNotifier * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IJusbPmpTransferNotifier * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IJusbPmpTransferNotifier * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IJusbPmpTransferNotifier * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IJusbPmpTransferNotifier * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IJusbPmpTransferNotifier * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IJusbPmpTransferNotifier * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCustomNotifier )( 
            IJusbPmpTransferNotifier * This,
            unsigned long long __MIDL__IJusbPmpTransferNotifier0000);
        
        END_INTERFACE
    } IJusbPmpTransferNotifierVtbl;

    interface IJusbPmpTransferNotifier
    {
        CONST_VTBL struct IJusbPmpTransferNotifierVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJusbPmpTransferNotifier_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IJusbPmpTransferNotifier_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IJusbPmpTransferNotifier_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IJusbPmpTransferNotifier_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IJusbPmpTransferNotifier_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IJusbPmpTransferNotifier_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IJusbPmpTransferNotifier_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IJusbPmpTransferNotifier_Cancel(This)	\
    ( (This)->lpVtbl -> Cancel(This) ) 

#define IJusbPmpTransferNotifier_SetCustomNotifier(This,__MIDL__IJusbPmpTransferNotifier0000)	\
    ( (This)->lpVtbl -> SetCustomNotifier(This,__MIDL__IJusbPmpTransferNotifier0000) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IJusbPmpTransferNotifier_INTERFACE_DEFINED__ */



#ifndef __JusbPmpComLib_LIBRARY_DEFINED__
#define __JusbPmpComLib_LIBRARY_DEFINED__

/* library JusbPmpComLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_JusbPmpComLib;

EXTERN_C const CLSID CLSID_CUsbPmpTransferNotifier;

#ifdef __cplusplus

class DECLSPEC_UUID("0F241DDE-1AF3-446e-A291-C6AE506EC9D4")
CUsbPmpTransferNotifier;
#endif
#endif /* __JusbPmpComLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


