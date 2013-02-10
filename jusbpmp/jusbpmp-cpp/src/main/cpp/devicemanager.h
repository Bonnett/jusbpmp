/*
 *	\file devicemanager.h
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
#ifndef DEVICEMANAGER_H_
#define DEVICEMANAGER_H_

#include <string>
#include <map>
#include <vector>

#include <dbt.h>
#include <Setupapi.h>
#include <cfgmgr32.h>
#include <winioctl.h>
#include <mswmdm.h>
#include <scclient.h>

interface IWMDMLogger;
class usbdevice;
class usbdevice_info;
class isyncnotifier;

class devicemanager
{
public:

	typedef std::map<std::wstring, usbdevice*> UsbDevices;
	typedef std::map<std::wstring, usbdevice_info> UsbDevicesInfo;
	
	devicemanager();
	virtual ~devicemanager();
	
	int 	check_device(std::wstring devid);
	int 	release_device(std::wstring devid);
	int 	has_device(std::wstring devid);
	int 	refresh_devices();
	int 	dump_devices();
	int 	fill_devicesinfo(UsbDevicesInfo* list);
	
	usbdevice* get_devicebyid(std::wstring devid);

	virtual void unset_mtp();
	virtual void setup_mtp();

private:
		
	int detect_devices_mtp();
	int detect_devices_msc();
	
	int clear_devices();
		

	CString				GetMtpDeviceCanonical(IWMDMDevice* pdevice);
	CString				GetMountPointForDevice(DWORD deviceNumber);
	DEVINST				GetDrivesDevInstByDeviceNumber(long DeviceNumber, UINT DriveType);
	CONFIGRET			GetDeviceName(DEVNODE DevNode, std::wstring& devName);
	//CONFIGRET			GetDeviceManuf(DEVNODE DevNode, CString& devManuf);
	//CString				GetDeviceSerialFromString(CString input, CString token, BOOL bType);
	//CString				GetProductIdFromString(CString input);
	//CString				GetVendorIdFromString(CString input);
	int					get_descriptor_props(std::wstring rootHubPath, std::wstring deviceUsbId, usbdevice* usbdev);
	USHORT				get_stringdescriptor(HANDLE hHubHandle, ULONG UsbPortIndex, USHORT LanguageID, UCHAR Index, std::wstring& descriptorValue);

	CSecureChannelClient	*m_pSAC; 
    IWMDeviceManager		*m_pWMDevMgr;
	CComPtr<IWMDMLogger>	m_spWMDMLogger;
	HRESULT					m_hrInit;
    IWMDMNotification		*m_pICallbackObject;
	DWORD					m_dwNotificationCookie;
	CRITICAL_SECTION		_criticalSection; 

	UsbDevices listDevices;
};

#endif /*DEVICEMANAGER_H_*/
