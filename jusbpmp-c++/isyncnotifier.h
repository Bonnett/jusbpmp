/*
 *	\file isyncnotifier.h
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
#ifndef ISYNCNOTIFIER_H_
#define ISYNCNOTIFIER_H_

class isyncnotifier
{
public:
	isyncnotifier();
	virtual ~isyncnotifier()=0;
	
	virtual long notify_begin(unsigned __int64 size) = 0;
	virtual long notify_current(unsigned __int64 pos) = 0;
	virtual long notify_end() = 0;
	virtual int  is_abort() = 0;
};

#endif /*ISYNCNOTIFIER_H_*/
