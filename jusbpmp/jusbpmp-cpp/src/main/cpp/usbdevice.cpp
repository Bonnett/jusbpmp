/*
 *	\file usbdevice.cpp
 *	Java Usb Device Management [libjpmp.so]
 *	Copyright (C) 2008  cmi [netcmi78@gmail.com]
 *	Based on Media Transfer Protocol library libmtp
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *	
 */
#include "stdafx.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif


#include "defines.h"

#include "usbdevice.h"
#include "helperlib.h"
#include "isyncnotifier.h"
#include "usbdevmanapp.h"
#include "jpmp_device_UsbDevice.h"
#include "filemetadata.h"

#include <mswmdm_i.c>
#include "transfernotifier.h"



#define MAX_BUF 1024	
usbdevice::usbdevice(){
	mode = MODE_UNDEF;
	idx = 0;
	mtp_device			= NULL;
	m_pStorageGlobals   = NULL;
    m_pEnumStorage      = NULL;
	m_pRootStorage.p	= NULL;
}

usbdevice::usbdevice(usbdevice* newval){
	this->canonical = newval->canonical;
	this->name = newval->name;
	this->serial = newval->serial;
	this->pid = newval->pid;
	this->vid = newval->vid;
	this->productid = newval->productid;
	this->vendorid = newval->vendorid;
	this->idx = newval->idx;
	this->mountpoint = newval->mountpoint;
	this->totalsize = newval->totalsize;
	this->freesize = newval->freesize;
	this->manufacturer = newval->manufacturer;
	this->videodir = newval->videodir;
	this->audiodir = newval->audiodir;
	this->pictdir = newval->pictdir;
	this->mtp_device = newval->mtp_device;
}

usbdevice::~usbdevice(){
    SAFE_RELEASE( m_pStorageGlobals );
    SAFE_RELEASE( m_pEnumStorage );
    SAFE_COMPTR_RELEASE( m_pRootStorage );
	SAFE_RELEASE(mtp_device);
}

int usbdevice::transfer_file(std::wstring src, std::wstring dest, isyncnotifier* pnotif, filemetadata* meta){
	int ret = 1;
	if (usbdevman_app::get_instance()->is_verbose() ){
		std::wcout << "usbdevice::transfer_file " << "to mountpoint " << mountpoint << std::endl;
	}
	if (this->mode == MODE_MSC){
		if (this->mountpoint.length()>0){
			std::wstring fulldest = this->mountpoint + dest;
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << "usbdevice::transfer_file " << src.c_str() << " to " << fulldest << std::endl;
			}
			return msc_copy(src.c_str(),fulldest.c_str(),pnotif);
		}
	} 
	if (this->mode == MODE_MTP){

		HRESULT							hr = S_OK;
		CComPtr<IWMDMStorageControl3>	spDestStorageControl = NULL;
		CComPtr<IWMDMStorage3>			spDestStorageFolder3 = NULL;
		CComPtr<IWMDMStorage>			spDestStorageFolder = NULL;
		CComPtr<IWMDMStorage>			spNewStorage = NULL;
		CComPtr<IWMDMStorage3>			spNewStorage3 = NULL;

		CComPtr<IWMDMStorage>			spTempStorage = NULL;
		CComPtr<IWMDMStorage>			spTempChildStorage = NULL;

		CComPtr<IWMDMMetaData>			spMeta;
		WMDM_TAG_DATATYPE				type;
		LPBYTE							lpValue = NULL;
		UINT							cbSize;

		CComPtr<IJusbPmpTransferNotifier> spProgress;
		CComPtr<IWMDMProgress>			spWMDMProgress;


		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << "usbdevice::transfer_file MTP " << src.c_str() << " to " << dest << std::endl;
		}
		
		std::wstring filename = dest;
		size_t idx = dest.find_last_of(L"/");
		if (idx != std::wstring::npos){
			filename = dest.substr(idx+1);
		}
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << "usbdevice::transfer_file MTP, destination " << filename << std::endl;
		}

		/*
		uint64_t fsize = helperlib::obtain_filesize(src.c_str());
		if (pnotif){
			pnotif->notify_begin(fsize);
		}*/

		// get progress notifier
		hr = CoCreateInstance(
			CLSID_CUsbPmpTransferNotifier,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IJusbPmpTransferNotifier,
			(void**)&spProgress);

		if (hr != S_OK){
			spProgress = NULL;
		}

		if (spProgress){
			spProgress->SetCustomNotifier((unsigned long long)pnotif);
			hr = spProgress->QueryInterface(&spWMDMProgress);
			if (!SUCCEEDED(hr)) { 
				spWMDMProgress = NULL;
			}
		}

			
			
		// retrieve sub folders names
		std::vector<std::wstring> vpath;
		helperlib::split_string(dest, L"/", vpath, false);
		
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << "getting subfolder for " << dest << std::endl;
			std::wcout << "getting subfolder , depth " << vpath.size() << std::endl;
		}

		try{

			// get folder tree
			hr =  m_pRootStorage.CopyTo(&spTempStorage);
			SUCCEEDED( hr ) ? 0 : throw hr;

			ret = 0;
			for (size_t i=0; i<vpath.size() - 1; i++){
				ret = get_substorage(spTempStorage, vpath[i].c_str(), TRUE, &spTempChildStorage);
				if( !SUCCEEDED( hr ) || spTempChildStorage.p==NULL) {
					if (usbdevman_app::get_instance()->is_verbose() ){
						std::wcout << L"sub storage " << vpath[i].c_str() << L" NOK" << std::endl;
					}
					SAFE_COMPTR_RELEASE(spTempStorage);
					SAFE_COMPTR_RELEASE(spTempChildStorage);
					throw hr;
				}
				SAFE_COMPTR_RELEASE(spTempStorage);
				spTempChildStorage.CopyTo(&spTempStorage);
				SAFE_COMPTR_RELEASE(spTempChildStorage);
			}

			if (spTempStorage.p==NULL) {
				throw E_FAIL;
			}
			hr = spTempStorage.CopyTo(&spDestStorageFolder);
			SAFE_COMPTR_RELEASE(spTempStorage);
			SUCCEEDED( hr ) ? 0 : throw hr;

			// we have the destination folder in spDestStorageFolder
			hr = spDestStorageFolder->QueryInterface(&spDestStorageFolder3);
			if (spDestStorageFolder3.p == NULL){
				throw E_FAIL;
			}

			hr = spDestStorageFolder3->QueryInterface(&spDestStorageControl);
			if (spDestStorageControl.p == NULL){
				throw E_FAIL;
			}

			
			hr = spDestStorageFolder3->CreateEmptyMetadataObject(&spMeta);
			if (hr != S_OK){
				SAFE_COMPTR_RELEASE(spMeta);
			}
			if (spMeta.p != NULL){
				LPWSTR title = (LPWSTR)meta->title.c_str();
				UINT numBytes = (UINT)( (wcslen(title) + 1) * sizeof(WCHAR) );
				hr = spMeta->AddItem(WMDM_TYPE_STRING, g_wszWMDMTitle, (BYTE*)title, numBytes);

				LPWSTR artist = (LPWSTR)meta->artist.c_str();
				numBytes = (UINT)( (wcslen(artist) + 1) * sizeof(WCHAR) );
				hr = spMeta->AddItem(WMDM_TYPE_STRING, g_wszWMDMAuthor, (BYTE*)artist, numBytes) ;

				LPWSTR album = (LPWSTR)meta->album.c_str();
				numBytes = (UINT)( (wcslen(album) + 1) * sizeof(WCHAR) );
				hr = spMeta->AddItem(WMDM_TYPE_STRING, g_wszWMDMAlbumArt, (BYTE*)album, numBytes) ;

				LPWSTR genre = (LPWSTR)meta->genre.c_str();
				numBytes = (UINT)( (wcslen(genre) + 1) * sizeof(WCHAR) );
				hr = spMeta->AddItem(WMDM_TYPE_STRING, g_wszWMDMGenre, (BYTE*)genre, numBytes) ;

				DWORD rating = meta->rating;
				hr = spMeta->AddItem(WMDM_TYPE_DWORD, g_wszWMDMUserRating, (BYTE*)&rating, sizeof(WMDM_TYPE_DWORD));

			}

			hr = spDestStorageControl->Insert3(
				 WMDM_MODE_BLOCK | WMDM_STORAGECONTROL_INSERTINTO | WMDM_CONTENT_FILE | WMDM_FILE_CREATE_OVERWRITE | WMDM_MODE_TRANSFER_PROTECTED | WMDM_MODE_TRANSFER_UNPROTECTED,
				 WMDM_FILE_ATTR_FILE,
				 //0,
				 (LPWSTR)src.c_str(), 
				 (LPWSTR)filename.c_str(),
                 NULL, 
				 spWMDMProgress, 
				 spMeta,
				 NULL,
                 &spNewStorage);

			SAFE_COMPTR_RELEASE(spMeta);
			if ( ( mode == MODE_MTP) && hr == S_OK){
				hr = spNewStorage->QueryInterface(&spNewStorage3);
				if (SUCCEEDED(hr)){		
					hr = spNewStorage3->GetMetadata(&spMeta);
					if (hr == S_OK){
						hr = spMeta->QueryByName(g_wszWMDMPersistentUniqueID, &type, &lpValue, &cbSize);
						if(SUCCEEDED(hr) && cbSize) {
							//storageUniqueId = CW2T((LPWSTR)lpValue);
							if (usbdevman_app::get_instance()->is_verbose() ){
								std::wcout << L" inserted itemID : " << (LPWSTR)lpValue << std::endl;
							}
						}
					}
				}
				SAFE_COMPTR_RELEASE(spMeta);
				SAFE_COMPTR_RELEASE(spNewStorage3);
				//continue even if error
				hr = S_OK;
				ret = 0;
			}


		} catch(HRESULT hrex){
			hr =  hrex;
			ret = hr;
		}

		// cleanup
		SAFE_COMPTR_RELEASE(spDestStorageFolder3);
		SAFE_COMPTR_RELEASE(spDestStorageFolder);
		SAFE_COMPTR_RELEASE(spNewStorage);
		SAFE_COMPTR_RELEASE(spTempChildStorage);
		SAFE_COMPTR_RELEASE(spTempStorage);
		SAFE_COMPTR_RELEASE(spDestStorageControl);
		SAFE_COMPTR_RELEASE(spWMDMProgress);
		SAFE_COMPTR_RELEASE(spProgress);
		SAFE_COMPTR_RELEASE(spMeta);

		/*if (ret == 0){
			if (pnotif){
				pnotif->notify_end();
			}
		}*/		
	}
	return ret;
}

#if 0
int usbdevice::get_substorage(LIBMTP_folder_t* folder, const char* name, uint32_t pid, uint32_t* foundid){
	int ret = 1;
	LIBMTP_folder_t* 	tempfolder = NULL;
	std::wstring 		filtername = name;
	
	if (usbdevman_app::get_instance()->is_verbose() ){
		//dump_folder(tempfolder,0);
		std::wcout << "call get_substorage name=" << name << ", parentid =" << pid << std::endl;
	}
	
	tempfolder = folder;
	//dump_folder_list(tempfolder,0);
	while (tempfolder){
		std::wstring foldername = tempfolder->name;
		if (tempfolder->parent_id == pid && foldername == filtername){
			// found the folder
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << "found the folder" << std::endl;
				//dump_folder(tempfolder,0);
			}			
			(*foundid) = tempfolder->folder_id;
			ret = 0;
			break;
		}
		if (tempfolder->child != NULL){
			ret = get_substorage(tempfolder->child,name,pid,foundid);
			if (ret == 0){
				break;
			}
		}
		tempfolder = tempfolder->sibling;
	}

	return ret;

}
#endif

int usbdevice::get_storage_bypath(LPCWSTR path, IWMDMStorage** ppStorageOut){
	int ret = 1;

	HRESULT							hr = S_OK;
	CComPtr<IWMDMStorage>			spTempStorage = NULL;
	CComPtr<IWMDMStorage>			spTempChildStorage = NULL;


	// check if root folder
	std::wstring wspath = path;
	if (wspath.length()==0 ){
		return jpmp_device_UsbDevice_ERROR_WRONG_PARAMETER;
	}

	if (wspath == L"/" || wspath == L"//" ){
		hr =  m_pRootStorage.CopyTo(ppStorageOut);
		if (hr == S_OK ){
			return jpmp_device_UsbDevice_ERROR_SUCCESS;
		}
		return jpmp_device_UsbDevice_ERROR_UNKNOWN;
	}


	// retrieve sub folders names
	std::vector<std::wstring> vpath;
	helperlib::split_string(path, L"/", vpath, false);
		
	if (usbdevman_app::get_instance()->is_verbose() ){
		//std::wcout << "getting subfolder for " << src << std::endl;
		//std::wcout << "getting subfolder , depth " << vpath.size() << std::endl;
	}
	try{

		// get folder tree
		hr =  m_pRootStorage.CopyTo(&spTempStorage);
		SUCCEEDED( hr ) ? 0 : throw hr;

		ret = 0;
		for (size_t i=0; i<vpath.size() ; i++){
			ret = get_substorage(spTempStorage, vpath[i].c_str(), FALSE, &spTempChildStorage);
			if( !SUCCEEDED( hr ) || spTempChildStorage.p==NULL) {
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"sub storage " << vpath[i].c_str() << L" NOK" << std::endl;
				}
				SAFE_COMPTR_RELEASE(spTempStorage);
				SAFE_COMPTR_RELEASE(spTempChildStorage);
				throw hr;
			}
			SAFE_COMPTR_RELEASE(spTempStorage);
			spTempChildStorage.CopyTo(&spTempStorage);
			SAFE_COMPTR_RELEASE(spTempChildStorage);
		}

		if (spTempStorage.p==NULL) {
			throw E_FAIL;
		}
		hr = spTempStorage.CopyTo(ppStorageOut);
		SAFE_COMPTR_RELEASE(spTempStorage);
		SUCCEEDED( hr ) ? 0 : throw hr;

		// we have the destination folder in ppStorageOut
		ret = 0;
	} catch(HRESULT hrex){
		hr = hrex;
		ret = 1;
	}

	// cleanup
	SAFE_COMPTR_RELEASE(spTempChildStorage);
	SAFE_COMPTR_RELEASE(spTempStorage);

	return ret;	

}

int usbdevice::get_substorage (IWMDMStorage* pStorageRoot, LPCWSTR folder, BOOL bCreate, IWMDMStorage** ppStorageOut){
	int ret = 1;

	HRESULT					hr = S_OK;
	IWMDMEnumStorage*		pEnumSubStorage = NULL;
	CComPtr<IWMDMStorage>	pTempStorage = NULL;
	ULONG					ulFetched;
	BOOL					bFound = FALSE;

	// check if pStorageRoot valid
	if (pStorageRoot == NULL) {
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << L"root storage is NULL" << std::endl;
		}			
		return 0;
	}

	try{
		hr = pStorageRoot->EnumStorage( &pEnumSubStorage );
		FAILED(hr) ? throw hr : 0;
		while(pEnumSubStorage ){
			pTempStorage = NULL;
			hr = pEnumSubStorage->Next( 1, &pTempStorage, &ulFetched );
			SUCCEEDED( hr ) ? 0 : throw hr;
			if (pTempStorage == NULL) {
				break;
			}
			WCHAR m_szTempName[MAX_PATH];
			hr = pTempStorage->GetName( m_szTempName, sizeof(m_szTempName)/sizeof(m_szTempName[0]) - 1 );
			if (!SUCCEEDED( hr )) {
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"Storage->GetName error : " << hr << std::endl;
				}
				throw hr;
			}

			CString strName(m_szTempName);
			strName.MakeLower();
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"found storage name : " << std::wstring(strName) << std::endl;
			}
			CString strFilter = folder;
			strFilter.MakeLower();
			if( strName == strFilter){
				//found
				hr = pTempStorage.CopyTo(ppStorageOut);
				if( !SUCCEEDED( hr )) {
					ppStorageOut = NULL;
					throw hr;
				}
				bFound = TRUE;
				ret = 0;
				SAFE_COMPTR_RELEASE(pTempStorage);
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"found storage name : " << std::wstring(folder) << std::endl;
				}
				break;
			}
			SAFE_COMPTR_RELEASE(pTempStorage);
		}
		if ((*ppStorageOut)==NULL && !bFound && bCreate){
			CString strmsg;
			strmsg.Format(L"creating folder %s on the device",folder);
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"found storage name : " << std::wstring(strmsg) << std::endl;
			}
			if (!pStorageRoot) throw hr;

			//if not, get that pointer
			IWMDMStorage* pTempStorage = NULL;
			hr = pStorageRoot->QueryInterface(IID_IWMDMStorage, (void**)&pTempStorage);
			if( !SUCCEEDED( hr )) {
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"Device down " << std::endl;
				}
				ppStorageOut = NULL;
				throw hr;
			}
			IWMDMStorageControl3* pTempControl = NULL;
			hr = pTempStorage->QueryInterface(IID_IWMDMStorageControl3, 
										   (void**)&pTempControl);
			if( !SUCCEEDED( hr )) {
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"Device down " << std::endl;
				}
				SAFE_RELEASE(pTempStorage);
				ppStorageOut = NULL;
				throw hr;
			}
				
			//create the folder on the device
			SAFE_RELEASE(pTempStorage);

			hr = pTempControl->Insert3( 
				//WMDM_MODE_BLOCK | WMDM_CONTENT_FOLDER |WMDM_FILE_CREATE_OVERWRITE,
				WMDM_MODE_BLOCK | WMDM_CONTENT_FOLDER ,
				WMDM_FILE_ATTR_FOLDER,
				NULL,
				(LPWSTR)folder,
				NULL,
				NULL,
				NULL,
				NULL,
				ppStorageOut);

			SAFE_RELEASE(pTempControl);

			Sleep(1000);

			if( !SUCCEEDED( hr )) {
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"Error Insert3 " << std::endl;
				}
				ppStorageOut = NULL;
				throw hr;
			}
			ret = 0;
		}
	} catch(HRESULT hrex){
		hr = hrex;
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << L"Error / catch " << hr << std::endl;
		}
		ret = hr;
	}

	SAFE_RELEASE(pEnumSubStorage);
	SAFE_COMPTR_RELEASE(pTempStorage);
	

	return ret;
}

#if 0
LIBMTP_folder_t* usbdevice::get_folderbyid(LIBMTP_folder_t* folderlist, uint32_t id){
	LIBMTP_folder_t* 	folder = NULL;
	LIBMTP_folder_t*	tempfolder;
	
	if (!this->mtp_device) return folder;
	if (folderlist == NULL){
		return folder;
	}
	
	tempfolder = folderlist;
	while (tempfolder){
		if (tempfolder->folder_id == id){
			// found the folder
			folder = tempfolder;
			break;
		}
		if (tempfolder->child != NULL){
			folder = get_folderbyid(tempfolder->child,id);
			if (folder != NULL){
				break;
			}
		}
		tempfolder = tempfolder->sibling;
	}
	return folder;
}
#endif

int usbdevice::check_make_dir( std::wstring path){
	std::wcout << "check_make_dir " << path << std::endl;
    if ( _waccess( path.c_str(), 0 ) == 0 ) {
        struct _stat status;
        if ( _wstat( path.c_str(), &status ) == 0){
		if ( status.st_mode & S_IFDIR ) {
			// ok, folder exists
			return 0;
	        } else {
			// nok, path exists, but is a file
			return 1;
        	}

	} else {
		//something wrong
		return 1;
	}
    } else {
        // no such path, try to create
		//return mkdir(path.c_str(),S_IXOTH);
		return _wmkdir(path.c_str());
    }
}

int usbdevice::make_dirs(std::wstring devid, std::wstring path, bool lastisfile){
	if (usbdevman_app::get_instance()->is_verbose() ){
		std::wcout << L"make_dir " << path << std::endl;
	}
	std::vector<std::wstring> vpath;
	helperlib::split_string(path, L"/", vpath, false);
		
	if (this->get_devicemode() == MODE_MSC){
		std::wstring fpath = this->get_mountpoint();
		// stop to size-1, if last item is the file
		int stop = ( (lastisfile ? 1 : 0) );
		for (size_t i=0; i<vpath.size() - stop; i++){
			fpath += L"/" + vpath[i];
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"current path = " << fpath << std::endl;
			}
			int ret = check_make_dir(fpath);
			if (ret != 0) return ret;
		}
	}
	if (this->get_devicemode() == MODE_MTP){
		return 0;
	}
	return 0;
}


bool usbdevice::delete_mtpitem(std::wstring fpath){
	int ret = 0;
	bool bResult = false;
	CComPtr<IWMDMStorage> spStorage;
	CComPtr<IWMDMStorageControl> spStorageControl;

	ret = get_storage_bypath(fpath.c_str(),&spStorage);
	if (ret == 0 ){
		HRESULT hr = spStorage->QueryInterface(&spStorageControl);
		if (hr == S_OK){
			hr = spStorageControl->Delete(WMDM_MODE_BLOCK, NULL);
		}
		bResult = (hr == S_OK);
	}
	SAFE_COMPTR_RELEASE(spStorageControl);
	SAFE_COMPTR_RELEASE(spStorage);
	return bResult;
}

bool usbdevice::exist_folder_mtp(std::wstring fpath){
	int ret = 0;
	bool bResult;
	CComPtr<IWMDMStorage> spStorage;

	ret = get_storage_bypath(fpath.c_str(),&spStorage);
	
	bResult = (ret==0 && spStorage.p!=NULL);
	if (bResult){
		DWORD dwAttrib;
		HRESULT hr = spStorage->GetAttributes(&dwAttrib, NULL);
		bResult = ((dwAttrib & WMDM_FILE_ATTR_FOLDER) == WMDM_FILE_ATTR_FOLDER);
	}
	SAFE_COMPTR_RELEASE(spStorage);
	
	return bResult;
}

bool usbdevice::exist_file_mtp(std::wstring fpath){
	
	int ret = 0;
	bool bResult;
	CComPtr<IWMDMStorage> spStorage;

	ret = get_storage_bypath(fpath.c_str(),&spStorage);
	
	bResult = (ret==0 && spStorage.p!=NULL);
	if (bResult){
		DWORD dwAttrib;
		HRESULT hr = spStorage->GetAttributes(&dwAttrib, NULL);
		bResult = ((dwAttrib & WMDM_FILE_ATTR_FILE) == WMDM_FILE_ATTR_FILE);
	}
	SAFE_COMPTR_RELEASE(spStorage);
	
	return bResult;
}

bool usbdevice::create_folder_mtp(std::wstring fpath){
	
	bool bResult = false;

	std::wstring folder_tocreate = L"";
	std::wstring folder_parent = L"";
	
	size_t pos = fpath.find_last_of(L"/");
	if (pos == std::wstring::npos) {
		folder_tocreate = L"";
	} else {
		folder_tocreate = fpath.substr(pos+1);
		folder_parent = fpath.substr(0,pos);
	}
	
	if (folder_tocreate.length()==0) return false;
	if (folder_parent.length()==0) return false;
	
	CComPtr<IWMDMStorage> spStorageParent;
	CComPtr<IWMDMStorageControl> spStorageParentControl;
	CComPtr<IWMDMStorage> spStorageNewFolder;

	int ret = get_storage_bypath(folder_parent.c_str(),&spStorageParent);
	if (ret == 0 ){
		HRESULT hr = spStorageParent->QueryInterface(&spStorageParentControl);
		if (hr == S_OK){
			hr = spStorageParentControl->Insert(
				WMDM_MODE_BLOCK | WMDM_CONTENT_FOLDER, 
				(LPWSTR)folder_tocreate.c_str(),
				NULL,
				NULL,
				&spStorageNewFolder);
		}
		bResult = (hr == S_OK);
	}
	SAFE_COMPTR_RELEASE(spStorageParentControl);
	SAFE_COMPTR_RELEASE(spStorageParent);
	SAFE_COMPTR_RELEASE(spStorageNewFolder);
	return bResult;
}

int usbdevice::parse_folder_msc(std::wstring path, mtp_itemlist* subfiles, mtp_itemlist* subfolders){
	int ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
	
	//get files of folder
	if (usbdevman_app::get_instance()->is_verbose() ){
		std::wcout << L"Check files in " << path << std::endl;
	}

	std::wstring devicepath = path;
	std::replace( devicepath.begin(), devicepath.end(), L'/', L'\\' );

	std::wstring fulldevicepath = get_mountpoint() + devicepath;

	bool b = helperlib::exist_folder_msc(fulldevicepath);
	if (!b) return jpmp_device_UsbDevice_ERROR_FOLDER_NOTFOUND;

	std::wstring match = fulldevicepath + L"\\*.*";
	WIN32_FIND_DATA ffData;
	HANDLE hFind;

	hFind = FindFirstFile(match.c_str(), &ffData);

	if (INVALID_HANDLE_VALUE == hFind){
		return jpmp_device_UsbDevice_ERROR_UNKNOWN;
	} 

	do {
		std::wstring fname = ffData.cFileName;
		if ( (ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY){
			if ( (fname == L".") && (fname == L"..") ){
				continue;
			}
			subfolders->push_back(make_pair(fname, L"0"));
		} else {
			subfiles->push_back(make_pair(fname, L"0"));
		}
	} while (FindNextFile(hFind, &ffData) != 0);
	
	if (hFind != INVALID_HANDLE_VALUE){
		FindClose(hFind);
	}
	
	return ret;
}

int usbdevice::parse_folder_mtp(std::wstring path, mtp_itemlist* subfiles, mtp_itemlist* subfolders){
	int ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
	
	//get files of folder
	if (usbdevman_app::get_instance()->is_verbose() ){
		std::wcout << L"Check files in " << path << std::endl;
	}

	HRESULT							hr = S_OK;
	CComPtr<IWMDMStorage>			spRootStorage = NULL;
	CComPtr<IWMDMEnumStorage>		spEnumSubStorage = NULL;
	CComPtr<IWMDMStorage>			spStorageFolder = NULL;
	CComPtr<IWMDMStorage3>			spStorageFolder3 = NULL;
	CComPtr<IWMDMMetaData>			spMeta;
	ULONG							ulFetched;
	DWORD							dwStorageAttributes;

	ret = get_storage_bypath(path.c_str(),&spRootStorage);
	if ( ret != 0) {
		SAFE_COMPTR_RELEASE(spRootStorage);
		return ret;
	}

	try{

		hr = spRootStorage->EnumStorage( &spEnumSubStorage );
		FAILED(hr) ? throw hr : 0;
		while(spEnumSubStorage ){
			SAFE_COMPTR_RELEASE(spStorageFolder);
			hr = spEnumSubStorage->Next( 1, &spStorageFolder, &ulFetched );
			SUCCEEDED( hr ) ? 0 : throw hr;
			if (spStorageFolder == NULL || spStorageFolder.p == NULL) {
				break;
			}

			WCHAR szWorkBuf[MAX_PATH];
			hr = spStorageFolder->GetName( szWorkBuf, sizeof(szWorkBuf)/sizeof(szWorkBuf[0]) - 1 );
			if (hr != S_OK){	
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"Storage->GetName error : " << hr << std::endl;
				}
				throw hr;
			}


			std::wstring storageid = L"0";
			std::wstring storagename = szWorkBuf;
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"found storage name : " << storagename << std::endl;
			}

			hr = spStorageFolder->QueryInterface(&spStorageFolder3);
			if (hr == S_OK){		
				hr = spStorageFolder3->GetMetadata(&spMeta);
				if (hr == S_OK){
					WMDM_TAG_DATATYPE	type;
					LPBYTE				lpValue = NULL;
					UINT				cbSize;

					hr = spMeta->QueryByName(g_wszWMDMPersistentUniqueID, &type, &lpValue, &cbSize);
					if(SUCCEEDED(hr) && cbSize) {
						storageid = (LPWSTR)lpValue;
						if (usbdevman_app::get_instance()->is_verbose() ){
							std::wcout << L" MTP itemID : " << storageid << std::endl;
						}
					}
					if (lpValue != NULL){
						CoTaskMemFree(lpValue);
					}
				}
				SAFE_COMPTR_RELEASE(spMeta);
			} else {
				throw hr;
			}

			hr = spStorageFolder3->GetAttributes(&dwStorageAttributes,NULL);
			SUCCEEDED( hr ) ? 0 : throw hr;

			if (WMDM_FILE_ATTR_FILE == (dwStorageAttributes & WMDM_FILE_ATTR_FILE) ){
				//this is a file
				subfiles->push_back(make_pair(storagename, storageid));
			}
			if (WMDM_FILE_ATTR_FOLDER == (dwStorageAttributes & WMDM_FILE_ATTR_FOLDER) ){
				//this is a folder
				subfolders->push_back(make_pair(storagename, storageid));
			}

			SAFE_COMPTR_RELEASE(spStorageFolder3);
			SAFE_COMPTR_RELEASE(spStorageFolder);
		}
	}catch (HRESULT hrex){
		hr = hrex;
		ret = 1;
	}

	SAFE_COMPTR_RELEASE(spStorageFolder);
	SAFE_COMPTR_RELEASE(spStorageFolder3);
	SAFE_COMPTR_RELEASE(spMeta);
	SAFE_COMPTR_RELEASE(spRootStorage);
	SAFE_COMPTR_RELEASE(spEnumSubStorage);
	

	return ret;
}

int usbdevice::get_metadata(std::wstring path, IWMDMMetaData** ppMetaOut){

	int								ret = 1;
	HRESULT							hr = S_OK;
	CComPtr<IWMDMStorage>			spStorage = NULL;
	CComPtr<IWMDMStorage3>			spStorage3 = NULL;

	ret = get_storage_bypath(path.c_str(),&spStorage);
	if (ret != 0){
		SAFE_COMPTR_RELEASE(spStorage);
		return ret;
	}

	try{

		// we have the destination folder in spStorage
		hr = spStorage->QueryInterface(&spStorage3);
		SUCCEEDED( hr ) ? 0 : throw hr;
		if (spStorage3.p == NULL){
			throw E_FAIL;
		}

		hr = spStorage3->GetMetadata(ppMetaOut);
		SUCCEEDED( hr ) ? 0 : throw hr;
		if (hr == S_OK){
			ret = 0;
		}
		
	} catch(HRESULT hrex){
		hr =  hrex;
		ret = hr;
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << "usbdevice::read_file MTP catch exception : " << hr << std::endl;
		}
	}

	// cleanup
	SAFE_COMPTR_RELEASE(spStorage3);
	SAFE_COMPTR_RELEASE(spStorage);

	return ret;
}

int usbdevice::update_metadata(std::wstring path, filemetadata* meta){
	if (this->mode != MODE_MTP){
		return jpmp_device_UsbDevice_ERROR_MTP_NOTAVAILABLE;
	}

	int								ret = 1;
	HRESULT							hr = S_OK;
	CComPtr<IWMDMStorage>			spStorage = NULL;
	CComPtr<IWMDMStorage3>			spStorage3 = NULL;
	CComPtr<IWMDMMetaData>			spMeta = NULL;

	ret = get_storage_bypath(path.c_str(),&spStorage);
	if (ret != 0){
		SAFE_COMPTR_RELEASE(spStorage);
		return ret;
	}

	try{

		// we have the destination folder in spStorage
		hr = spStorage->QueryInterface(&spStorage3);
		SUCCEEDED( hr ) ? 0 : throw hr;
		if (spStorage3.p == NULL){
			throw E_FAIL;
		}

		hr = spStorage3->GetMetadata(&spMeta);
		SUCCEEDED( hr ) ? 0 : throw hr;

		if (spMeta.p == NULL){
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"create empty metadata " << std::endl;
			}
			hr = spStorage3->CreateEmptyMetadataObject(&spMeta);
		}

		if (spMeta.p != NULL){
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"fill metadata properties" << std::endl;
			}
			LPWSTR title = (LPWSTR)meta->title.c_str();
			UINT numBytes = (UINT)((wcslen(title) + 1) * sizeof(WCHAR));
			hr = spMeta->AddItem(WMDM_TYPE_STRING, g_wszWMDMTitle, (BYTE*)title, numBytes);
			if ( hr != S_OK){
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"cannot set metadata property g_wszWMDMTitle as " << title << std::endl;
				}
			}

			LPWSTR artist = (LPWSTR)meta->artist.c_str();
			numBytes = (UINT)((wcslen(artist) + 1) * sizeof(WCHAR));
			hr = spMeta->AddItem(WMDM_TYPE_STRING, g_wszWMDMAuthor, (BYTE*)artist, numBytes) ;
			if ( hr != S_OK){
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"cannot set metadata property g_wszWMDMAuthor as " << artist << std::endl;
				}
			}

			LPWSTR album = (LPWSTR)meta->album.c_str();
			numBytes = (UINT)((wcslen(album) + 1) * sizeof(WCHAR));
			hr = spMeta->AddItem(WMDM_TYPE_STRING, g_wszWMDMAlbumArt, (BYTE*)album, numBytes) ;
			if ( hr != S_OK){
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"cannot set metadata property g_wszWMDMAlbumArt as " << album << std::endl;
				}
			}

			LPWSTR genre = (LPWSTR)meta->genre.c_str();
			numBytes = (UINT)((wcslen(genre) + 1) * sizeof(WCHAR));
			hr = spMeta->AddItem(WMDM_TYPE_STRING, g_wszWMDMGenre, (BYTE*)genre, numBytes) ;
			if ( hr != S_OK){
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"cannot set metadata property g_wszWMDMGenre as " << genre << std::endl;
				}
			}

			DWORD rating = meta->rating;
			hr = spMeta->AddItem(WMDM_TYPE_DWORD, g_wszWMDMUserRating, (BYTE*)&rating, sizeof(WMDM_TYPE_DWORD));
			if ( hr != S_OK){
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L"cannot set metadata property g_wszWMDMUserRating as " << rating << std::endl;
				}
			}

		}
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << L"set metadata" << std::endl;
		}
		hr = spStorage3->SetMetadata(spMeta);
		if (hr == S_OK){
			ret = 0;
		} else {
			throw hr;
		}
		
	} catch(HRESULT hrex){
		hr =  hrex;
		ret = hr;
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << "usbdevice::read_file MTP catch exception : " << hr << std::endl;
		}
	}

	// cleanup
	SAFE_COMPTR_RELEASE(spStorage3);
	SAFE_COMPTR_RELEASE(spStorage);
	SAFE_COMPTR_RELEASE(spMeta);

	return ret;	

}

int usbdevice::read_file(std::wstring src, std::wstring dest, isyncnotifier* pnotif){
	int ret = 1;
	if (this->mode == MODE_MSC){
		if (this->mountpoint.length()>0){
			std::wstring fullsrc = this->mountpoint + src;
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << "usbdevice::read_file " << fullsrc.c_str() << " to " << dest << std::endl;
			}
			return msc_copy(fullsrc.c_str(),dest.c_str(),pnotif);
		}
	} 
	if (this->mode == MODE_MTP){
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << "usbdevice::read_file MTP " << src.c_str() << " to " << dest << std::endl;
		}
		HRESULT							hr = S_OK;
		CComPtr<IWMDMStorageControl3>	spDestStorageControl = NULL;
		CComPtr<IWMDMStorage3>			spDestStorageFolder3 = NULL;
		CComPtr<IWMDMStorage>			spDestStorageFolder = NULL;
		CComPtr<IWMDMStorage>			spNewStorage = NULL;
		CComPtr<IWMDMStorage3>			spNewStorage3 = NULL;

		CComPtr<IWMDMStorage>			spTempStorage = NULL;
		CComPtr<IWMDMStorage>			spTempChildStorage = NULL;

		CComPtr<IWMDMMetaData>			spMeta;
		LPBYTE							lpValue = NULL;
			
		CComPtr<IJusbPmpTransferNotifier> spProgress;
		CComPtr<IWMDMProgress>			spWMDMProgress;


		std::wstring filename = src;
		size_t idx = dest.find_last_of(L"/");
		if (idx != std::wstring::npos){
			filename = dest.substr(idx+1);
		}
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << "usbdevice::read_file MTP, source " << filename << std::endl;
		}
		
		// retrieve sub folders names
		std::vector<std::wstring> vpath;
		helperlib::split_string(src, L"/", vpath, false);
		
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << "getting subfolder for " << src << std::endl;
			std::wcout << "getting subfolder , depth " << vpath.size() << std::endl;
		}

		// get progress notifier
		hr = CoCreateInstance(
			CLSID_CUsbPmpTransferNotifier,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IJusbPmpTransferNotifier,
			(void**)&spProgress);

		if (hr != S_OK){
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << "usbdevice::read_file MTP cannot CoCreateInstance for progress notifier " << std::endl;
			}
			spProgress = NULL;
		}

		if (spProgress){
			spProgress->SetCustomNotifier((unsigned long long)pnotif);
			hr = spProgress->QueryInterface(&spWMDMProgress);
			if (!SUCCEEDED(hr)) { 
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << "usbdevice::read_file MTP cannot QueryInterface for progress notifier " << std::endl;
				}
				spWMDMProgress = NULL;
			}
		}

		try{

			// get folder tree
			hr =  m_pRootStorage.CopyTo(&spTempStorage);
			SUCCEEDED( hr ) ? 0 : throw hr;

			ret = 0;
			for (size_t i=0; i<vpath.size() ; i++){
				ret = get_substorage(spTempStorage, vpath[i].c_str(), FALSE, &spTempChildStorage);
				if( !SUCCEEDED( hr ) || spTempChildStorage.p==NULL) {
					if (usbdevman_app::get_instance()->is_verbose() ){
						std::wcout << L"sub storage " << vpath[i].c_str() << L" NOK" << std::endl;
					}
					SAFE_COMPTR_RELEASE(spTempStorage);
					SAFE_COMPTR_RELEASE(spTempChildStorage);
					throw hr;
				}
				SAFE_COMPTR_RELEASE(spTempStorage);
				spTempChildStorage.CopyTo(&spTempStorage);
				SAFE_COMPTR_RELEASE(spTempChildStorage);
			}

			if (spTempStorage.p==NULL) {
				throw E_FAIL;
			}
			hr = spTempStorage.CopyTo(&spDestStorageFolder);
			SAFE_COMPTR_RELEASE(spTempStorage);
			SUCCEEDED( hr ) ? 0 : throw hr;

			// we have the destination folder in spDestStorageFolder
			hr = spDestStorageFolder->QueryInterface(&spDestStorageFolder3);
			if (spDestStorageFolder3.p == NULL){
				throw E_FAIL;
			}

			hr = spDestStorageFolder3->QueryInterface(&spDestStorageControl);
			if (spDestStorageControl.p == NULL){
				throw E_FAIL;
			}
			
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << "usbdevice::read_file MTP prepare to read " << filename << std::endl;
			}
			hr = spDestStorageControl->Read(
				 WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,
				 (LPWSTR)dest.c_str(),
				 spWMDMProgress, 
				 NULL);

			
		} catch(HRESULT hrex){
			hr =  hrex;
			ret = hr;
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << "usbdevice::read_file MTP catch exception : " << hr << std::endl;
			}
		}

		// cleanup
		SAFE_COMPTR_RELEASE(spDestStorageFolder3);
		SAFE_COMPTR_RELEASE(spDestStorageFolder);
		SAFE_COMPTR_RELEASE(spNewStorage);
		SAFE_COMPTR_RELEASE(spTempChildStorage);
		SAFE_COMPTR_RELEASE(spTempStorage);
		SAFE_COMPTR_RELEASE(spDestStorageControl);
		SAFE_COMPTR_RELEASE(spWMDMProgress);
		SAFE_COMPTR_RELEASE(spProgress);

		/*if (ret == 0){
			if (pnotif){
				pnotif->notify_end();
			}
		}*/		
	}
	return ret;
}

int usbdevice::msc_copy(LPCWSTR src, LPCWSTR dest, isyncnotifier* pnotif){
	FILE* f_from;		/* stream of source file. */
	FILE* f_to;			/* stream of target file. */
	char buf[MAX_BUF];   /* input buffer.          */
	int retcode = 0;

	/* open the source and the target files. */
	f_from = _wfopen(src, L"rb");
	if (!f_from) {
		fwprintf(stderr, L"Cannot open source file: %s\n", src);
		return 1;
	}
	f_to = _wfopen(dest, L"wb");
	if (!f_from) {
		fwprintf(stderr, L"Cannot open target file: %s\n",dest);
		return 2;
	}

	fseek(f_from,0,SEEK_END);
	long fileSize = ftell(f_from);
	fseek(f_from,0,SEEK_SET);
	size_t crtCopied = 0;
	size_t bytesread = 0;
	size_t byteswritten = 0;

	if (pnotif){
		pnotif->notify_begin(fileSize);
	}

	while(1){
		if (pnotif && pnotif->is_abort()){
			break;
		}
		memset(buf,0x0,MAX_BUF);
	    bytesread = fread(buf, 1, MAX_BUF, f_from);
	    if (bytesread==0){
	    	break;
	    }
	    crtCopied += bytesread;
		byteswritten = fwrite(buf, 1, bytesread, f_to);
		if (byteswritten!=bytesread) {  /* error writing data */
			fprintf(stderr, "Error writing to target file: %s\n",dest);
			retcode = 3;
			break;
		}
		if (pnotif){
			pnotif->notify_current((long)crtCopied);
		}
	}
	if (pnotif){
		pnotif->notify_end();
	}
	if (!feof(f_from)) { /* fgets failed _not_ due to encountering EOF */
		fprintf(stderr, "Error reading from source file: %s\n", src);
		return 4;
	}

	/* close source and target file streams. */
	if (fclose(f_from) == EOF) {
		fprintf(stderr, "Error when closing source file: %s\n", src);
	}
	if (fclose(f_to) == EOF) {
		fprintf(stderr, "Error when closing target file: %s\n",dest);
	}
	return 0;
}

int usbdevice::mtp_copy_progress (uint64_t const sent, uint64_t const total, void const *const data) {
	isyncnotifier* pnotif = (isyncnotifier*)data;
	if (pnotif){
		pnotif->notify_current((long)sent);
	}
	if (pnotif){
		if (pnotif->is_abort()){
			return 1;
		}
	}		
	return 0;	
}

HRESULT usbdevice::init_mtp( IWMDMDevice *pDevice ){
    HRESULT					hr;
    ULONG					ulFetched;
	IWMDMEnumStorage		*pEnumRootStorage;
    CComPtr<IWMDMDevice3>	spIDevice3;
    PROPVARIANT				propvar;
	WCHAR					szWorkBuf[MAX_PATH];

	try {

		if (pDevice == NULL){
			//logger->error(_T("pDevice == NULL"), __WFILE_LINE__);
			return E_ABORT;
		}

		// Get the RootStorage, SotrageGlobals, and EnumStorage interfaces
		m_pRootStorage    = NULL;
		m_pEnumStorage    = NULL;
		m_pStorageGlobals = NULL;

		hr = pDevice->EnumStorage( &pEnumRootStorage );
		SUCCEEDED( hr ) ? 0 : throw hr;
		if (!pEnumRootStorage) throw E_FAIL;

		hr = pEnumRootStorage->Next( 1, &m_pRootStorage, &ulFetched );
		SUCCEEDED( hr ) ? 0 : throw hr;
		if (!m_pRootStorage) throw E_FAIL;

		hr = m_pRootStorage->GetStorageGlobals( &m_pStorageGlobals );
		SUCCEEDED( hr ) ? 0 : throw hr;
		if (!m_pStorageGlobals) throw E_FAIL;

		hr = m_pRootStorage->EnumStorage( &m_pEnumStorage );
		SUCCEEDED( hr ) ? 0 : throw hr;
		if (!m_pEnumStorage) throw E_FAIL;

		SAFE_RELEASE(pEnumRootStorage);

		// Get device name
		hr = pDevice->GetName( szWorkBuf, sizeof(szWorkBuf)/sizeof(szWorkBuf[0]) - 1 );
		if( FAILED(hr) ) {
			 wcscpy_s( szWorkBuf, L"");
		}
		name = szWorkBuf;

		//Get canonical name
		IWMDMDevice2* pTempDevice2;
		hr = pDevice->QueryInterface(IID_IWMDMDevice2, (void**)&pTempDevice2);
		if( FAILED(hr) ) {
			 wcscpy_s( szWorkBuf, L"");
		} else{
			hr = pTempDevice2->GetCanonicalName( szWorkBuf, sizeof(szWorkBuf)/sizeof(szWorkBuf[0]) - 1 );
			if( FAILED(hr) ){
				 wcscpy_s( szWorkBuf, L"");
			}
			SAFE_RELEASE(pTempDevice2);
		}
		canonical = szWorkBuf;
		

		// Set the device pointer and addref it
		mtp_device = pDevice;
		//mtp_device->AddRef();

		// Get device type
		/*
		hr = mtp_device->GetType( &m_dwType );
		if( FAILED(hr) ) {
			m_dwType = 0L;
		}*/

		/*
		/// Get device serial number
		BYTE abMAC[SAC_MAC_LEN];
		BYTE abMACVerify[SAC_MAC_LEN];
		HMAC hMACVerify;

		hr = mtp_device->GetSerialNumber( &m_SerialNumber, (BYTE*)abMAC );
		if( SUCCEEDED(hr) ) {
			m_pDevManager->m_pSAC->MACInit(&hMACVerify);
			m_pDevManager->m_pSAC->MACUpdate(hMACVerify, (BYTE*)(&m_SerialNumber), sizeof(m_SerialNumber));
			m_pDevManager->m_pSAC->MACFinal(hMACVerify, (BYTE*)abMACVerify);
			if( memcmp(abMACVerify, abMAC, sizeof(abMAC)) != 0 ) {
				hr = E_FAIL;
			}
		}
		if( FAILED(hr) ) {
			FillMemory( (void *)&m_SerialNumber, sizeof(m_SerialNumber), 0 );
		}
		*/

		// Get device manufacturer
		hr = mtp_device->GetManufacturer( szWorkBuf, sizeof(szWorkBuf)/sizeof(szWorkBuf[0]) - 1 );
		if( FAILED(hr) ) {
			wcscpy_s( szWorkBuf, L"");
		}
		manufacturer = szWorkBuf;


		//
		//Get device properties through GetProperty method
		//
		PropVariantInit (&propvar);
		hr = mtp_device->QueryInterface (__uuidof(IWMDMDevice3), reinterpret_cast<void**>(&spIDevice3));
		if (SUCCEEDED (hr)){

			CComBSTR  bstrDeviceProp;

			/*
			//
			//Get g_wszWMDMDeviceFriendlyName
			//
			hr = spIDevice3->GetProperty (g_wszWMDMDeviceFriendlyName, &propvar);
			if (SUCCEEDED (hr) && (VT_BSTR == propvar.vt)) {
				bstrDeviceProp.Attach(propvar.bstrVal);
			}
			name = std::wstring(CString(bstrDeviceProp));
			propvar.vt = VT_EMPTY;


			//
			//Get g_wszWMDMDeviceModelName
			//
			hr = spIDevice3->GetProperty (g_wszWMDMDeviceModelName, &propvar);
			if (SUCCEEDED (hr) && (VT_BSTR == propvar.vt)) {
				bstrDeviceProp.Attach(propvar.bstrVal);
			}
			name = std::wstring(CString(bstrDeviceProp));
			propvar.vt = VT_EMPTY;			

			//
			//Get g_wszWMDMDeviceVendorExtension to set m_bstrVendorExtDesc
			//
			hr = spIDevice3->GetProperty (g_wszWMDMDeviceVendorExtension, &propvar);
			if (SUCCEEDED (hr) && (VT_BSTR == propvar.vt)) {
				bstrDeviceProp.Attach(propvar.bstrVal);
			}
			name = std::wstring(CString(bstrDeviceProp));
			propvar.vt = VT_EMPTY; //since we have transferred BSTR ownsership to m_bstrVendorExtDesc;
			*/

			//
			//Get g_wszWMDMDeviceProtocol to set m_deviceProtocol
			//
			hr = spIDevice3->GetProperty(g_wszWMDMDeviceProtocol, &propvar);
			if (SUCCEEDED(hr) && (VT_CLSID == propvar.vt)) {
				CLSID deviceProtocol = *propvar.puuid;
				if (deviceProtocol == WMDM_DEVICE_PROTOCOL_MTP){
					mode = MODE_MTP;
					
				}
			}
			PropVariantClear(&propvar); //clear the propvariant obtained
			
	        /*
			//
			//Get g_wszWMDMDeviceServiceProviderVendor to set m_deviceSPVendor
			//
			hr = spIDevice3->GetProperty(g_wszWMDMDeviceServiceProviderVendor, &propvar);
			if (SUCCEEDED(hr) && (VT_CLSID == propvar.vt))
			{
				m_deviceSPVendor= *propvar.puuid;
			}
			PropVariantClear(&propvar); //clear the propvariant obtained
			*/

		}

		// Get the total, free, and bad space on the storage
		//
		{
			DWORD dwLow;
			DWORD dwHigh;

			totalsize = 0;
			hr = m_pStorageGlobals->GetTotalSize( &dwLow, &dwHigh );
			if( SUCCEEDED(hr) )	{
				INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

				totalsize = (DWORD)nSize;
			}

			/*
			m_dwMemBadKB = 0;
			hr = m_pStorageGlobals->GetTotalBad( &dwLow, &dwHigh );
			if( SUCCEEDED(hr) )
			{
				INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

				m_dwMemBadKB = (DWORD)nSize;
			}*/

			freesize = 0;
			hr = m_pStorageGlobals->GetTotalFree( &dwLow, &dwHigh );
			if( SUCCEEDED(hr) ){
				INT64 nSize = ( (INT64)dwHigh << 32 | (INT64)dwLow ) >> 10;

				freesize = (DWORD)nSize;
			}
		}

		hr = S_OK;
	} catch(HRESULT hrex){
		hr = hrex;
	}

    return hr;
}

