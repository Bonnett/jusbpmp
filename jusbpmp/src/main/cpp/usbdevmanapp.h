/*
 *	\file usbdevmanapp.h
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
#ifndef USBDEVMANAPP_H_
#define USBDEVMANAPP_H_

class usbdevman_app{

private:
	static usbdevman_app* _instance;
	
	usbdevman_app();
	virtual ~usbdevman_app();
	
	int verbose;
	
public:
	static usbdevman_app* get_instance();
	static void release_instance();
	
	int is_verbose()			{return this->verbose;};
	void set_verbose(int val)	{this->verbose = val;}
	
};

#endif /*USBDEVMANAPP_H_*/
