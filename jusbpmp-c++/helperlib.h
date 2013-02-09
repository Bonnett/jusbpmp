/*
 *	\file helperlib.h
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
#ifndef HELPERLIB_H_
#define HELPERLIB_H_

#include <vector>

class helperlib
{
public:
	static uint64_t 			obtain_filesize(std::wstring path);
	//static LIBMTP_filetype_t 	find_filetype (std::wstring filename);
	static int 					split_string(const std::wstring& input, const std::wstring& delimiter, std::vector<std::wstring>& results, bool includeempties);
	static bool					exist_file_msc(std::wstring path);
	static bool					exist_folder_msc(std::wstring path);
};

#endif /*HELPERLIB_H_*/
