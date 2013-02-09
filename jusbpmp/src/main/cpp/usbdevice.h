/*
 *	\file usbdevice.h
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
#ifndef USBDEVICE_H_
#define USBDEVICE_H_


#include <string>
#include <vector>

#include <mswmdm.h>
#include <scclient.h>
#include "usbdevice_info.h"

class isyncnotifier;
class filemetadata;


interface IWMDMStorageGlobals;
interface IWMDMEnumStorage;
interface IWMDMStorage;
interface IWMHeaderInfo;
interface IWMDMDevice;
interface IComponentAuthenticate;

class usbdevice : public usbdevice_info
{
public:

	typedef std::pair<std::wstring,std::wstring> mtp_iteminfo;
	typedef std::vector<mtp_iteminfo> mtp_itemlist;

	usbdevice();
	usbdevice(usbdevice* newval);
	virtual ~usbdevice();
	
	
	void set_mtpdevice(IWMDMDevice* newval)		{this->mtp_device = newval;};
	
	IWMDMDevice* get_mtpdevice()				{return mtp_device;};
	
	int transfer_file(std::wstring src, std::wstring dest, isyncnotifier* pnotif, filemetadata* meta);
	int read_file(std::wstring src, std::wstring dest, isyncnotifier* pnotif);

	int get_metadata(std::wstring path, IWMDMMetaData** ppMetaOut);
	int update_metadata(std::wstring path, filemetadata* meta);

	bool delete_mtpitem(std::wstring fpath);
	bool create_folder_mtp(std::wstring fpath);
	bool exist_folder_mtp(std::wstring fpath);
	bool exist_file_mtp(std::wstring fpath);
	int parse_folder_mtp(std::wstring path, mtp_itemlist* subfiles, mtp_itemlist* subfolders);
	int parse_folder_msc(std::wstring path, mtp_itemlist* subfiles, mtp_itemlist* subfolders);
	
	HRESULT init_mtp( IWMDMDevice *pDevice );
	
	
protected:

	// MTP properties
	IWMDMStorageGlobals		*m_pStorageGlobals;
    IWMDMEnumStorage		*m_pEnumStorage;
	IWMDMDevice				*mtp_device;
	CComPtr<IWMDMStorage>	m_pRootStorage;
	WMDMID					m_SerialNumber;

	
	
	
	int make_dirs(std::wstring devid, std::wstring path, bool lastisfile);
	int check_make_dir( std::wstring );

	int get_storage_bypath(LPCWSTR path, IWMDMStorage** ppStorageOut);
	int get_substorage(IWMDMStorage* pStorageRoot, LPCWSTR folder, BOOL bCreate, IWMDMStorage** ppStorageOut);

	int msc_copy(LPCWSTR src, LPCWSTR dest, isyncnotifier* pnotif);
	static int mtp_copy_progress (uint64_t const sent, uint64_t const total, void const *const data);

};

#endif /*USBDEVICE_H_*/
