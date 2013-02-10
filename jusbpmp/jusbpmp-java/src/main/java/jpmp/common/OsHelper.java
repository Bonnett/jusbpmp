/*
 *	Java Usb Device Management
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
package jpmp.common;

public class OsHelper {
	
	private static final Integer OS_WINDOWS = new Integer(1);
	private static final Integer OS_LINUX = new Integer(2);
	private static final Integer OS_MAC = new Integer(3);
	
	private static Integer OS_CURRENT = null; 
	
	public static void checkOs(){
		if (OS_CURRENT != null) return;
		String osname = System.getProperty("os.name").toLowerCase();
		if (osname.startsWith("linux")){
			OS_CURRENT = OS_LINUX;
			return;
		}
		if (osname.startsWith("windows")){
			OS_CURRENT = OS_WINDOWS;
			return;			
		}
		if (osname.startsWith("mac")){
			OS_CURRENT = OS_MAC;
			return;			
		}
	}
	
	public static boolean isLinux(){
		checkOs();
		return OS_LINUX.equals(OS_CURRENT);
	}
	public static boolean isWindows(){
		checkOs();
		return OS_WINDOWS.equals(OS_CURRENT);
	}
	public static boolean isMac(){
		checkOs();
		return OS_MAC.equals(OS_CURRENT);
	}
	

}
