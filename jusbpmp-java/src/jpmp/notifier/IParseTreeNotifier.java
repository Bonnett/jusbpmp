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
package jpmp.notifier;

/**
 * Callback for USB device directory structure parsing 
 * 
 * @author cmi
 * <p>
 * This is the callback for 'parseFolder' method of class UsbDevice
 * </p>
 *
 */
public interface IParseTreeNotifier {

	/**
	 * @param folderName name of sub folder found
	 * @param mtpItemIid MTP item ID for MTP connected devices, for MSC this is always 0
	 * @return 0 if success
	 */
	public long addFolder(String folderName, String mtpItemIid);
	/**
	 * 
	 * @param fileName name of the file found
	 * @param mtpItemIid MTP item ID for MTP connected devices, for MSC this is always 0
	 * @return 0 if success
	 */
	public long addFile(String fileName, String mtpItemIid);
}
