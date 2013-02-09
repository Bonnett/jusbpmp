/*
 *	\file usbdevice_info.h
 *	Java Usb Device Management [libjusbpmp.so]
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
#ifndef USBDEVICE_INFO_H_
#define USBDEVICE_INFO_H_

#include <string>
#include <vector>

class usbdevice_info
{
public:

	typedef enum {MODE_UNDEF = 0, MODE_MSC, MODE_MTP} enumDeviceModes;

	usbdevice_info();
	usbdevice_info(usbdevice_info* newval);
	usbdevice_info(const usbdevice_info& newval);
	virtual ~usbdevice_info();
	
	
	void set_canonical(std::wstring newval)		{this->canonical = newval;};
	void set_name(std::wstring newval)			{this->name = newval;};
	void set_serial(std::wstring newval)			{this->serial = newval;};
	void set_pid(std::wstring newval)			{this->pid = newval;};
	void set_vid(std::wstring newval)			{this->vid = newval;};
	void set_productid(int newval)				{this->productid = newval;};
	void set_vendorid(int newval)				{this->vendorid = newval;};
	void set_idx(int newval)					{this->idx = newval;};
	void set_mountpoint(std::wstring newval)		{this->mountpoint = newval;};
	void set_totalsize(uint64_t newval)			{this->totalsize = newval;};
	void set_freesize(uint64_t newval)			{this->freesize = newval;};
	void set_manufacturer(std::wstring newval)	{this->manufacturer = newval;};
	void set_videodir(std::wstring newval)		{this->videodir = newval;};
	void set_audiodir(std::wstring newval)		{this->audiodir = newval;};
	void set_pictdir(std::wstring newval)		{this->pictdir = newval;};
	void set_devicemode(enumDeviceModes newval)	{this->mode = newval;};
	
	
	std::wstring get_canonical()				{return canonical;};
	std::wstring get_name()						{return name;};
	std::wstring get_serial()					{return serial;};
	std::wstring get_pid()						{return pid;};
	std::wstring get_vid()						{return vid;};
	int 		get_productid()					{return this->productid;};
	int 		get_vendorid()					{return this->vendorid;};
	int			get_idx()						{return idx;};
	std::wstring get_mountpoint()				{return mountpoint;};
	uint64_t	get_totalsize()					{return totalsize;};
	uint64_t	get_freesize()					{return freesize;};
	std::wstring get_manufacturer()				{return manufacturer;};
	std::wstring get_videodir()					{return videodir;};
	std::wstring get_audiodir()					{return audiodir;};
	std::wstring get_pictdir()					{return pictdir;};
	enumDeviceModes get_devicemode()			{return mode;};
	
	void dump();
	
protected:
	std::wstring canonical;
	std::wstring name;
	std::wstring serial;
	std::wstring pid;
	std::wstring vid;
	int			productid;
	int			vendorid;
	std::wstring mountpoint;
	std::wstring manufacturer;
	std::wstring videodir;
	std::wstring audiodir;
	std::wstring pictdir;
	int			idx;
	uint64_t	totalsize;
	uint64_t	freesize;
	enumDeviceModes mode;
	
};
#endif
