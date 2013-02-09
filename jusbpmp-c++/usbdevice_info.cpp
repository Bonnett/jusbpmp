/*
 *	\file usbdevice_info.cpp
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

#include "stdafx.h"
#include <iostream>
#include <sys/stat.h>

#include "usbdevice_info.h"
#include "usbdevmanapp.h"

usbdevice_info::usbdevice_info(){
	mode = MODE_UNDEF;
	idx = 0;
}

usbdevice_info::usbdevice_info(usbdevice_info* newval){
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
}

usbdevice_info::usbdevice_info(const usbdevice_info& newval){
	this->canonical = newval.canonical;
	this->name = newval.name;
	this->serial = newval.serial;
	this->pid = newval.pid;
	this->vid = newval.vid;
	this->productid = newval.productid;
	this->vendorid = newval.vendorid;
	this->idx = newval.idx;
	this->mountpoint = newval.mountpoint;
	this->totalsize = newval.totalsize;
	this->freesize = newval.freesize;
	this->manufacturer = newval.manufacturer;
	this->videodir = newval.videodir;
	this->audiodir = newval.audiodir;
	this->pictdir = newval.pictdir;
}

usbdevice_info::~usbdevice_info(){
}

void usbdevice_info::dump(){
	if (usbdevman_app::get_instance()->is_verbose() ){
		std::wcout << L"============================= " << std::endl;
		std::wcout << L"canonical = " << canonical << std::endl;
		std::wcout << L"serial = " << serial << std::endl;
		std::wcout << L"pid = " << pid << std::endl;
		std::wcout << L"vid = " << vid << std::endl;
		std::wcout << L"idx = " << idx << std::endl;
		std::wcout << L"totalsize = " << totalsize << std::endl;
		std::wcout << L"freesize = " << freesize << std::endl;
		std::wcout << L"mountpoint = " << mountpoint << std::endl;
		std::wcout << L"manufacturer = " << manufacturer << std::endl;
		std::wcout << L"videodir = " << videodir << std::endl;
		std::wcout << L"audiodir = " << audiodir << std::endl;
		std::wcout << L"pictdir = " << pictdir << std::endl;
		std::wcout << L"mode = " << mode << std::endl;
		std::wcout << L"============================= " << std::endl;
	}
}

