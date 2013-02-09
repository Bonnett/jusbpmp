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
 * File transfer progress notifier 
 * 
 * @author cmi
 * <p>Implement this interface to get notified of file transfer progress (read / write) </p>
 *
 */
public interface IDeviceTransferNotifier {
	
	/**
	 * Receive the file size that is transferred
	 * @param estimatedSize
	 */
	public void notifyBegin(long estimatedSize);
	/**
	 * Receive currently transferred size
	 * @param position
	 */
	public void notifyCurrent(long position);
	/**
	 * transfer ended
	 */
	public void notifyEnd();
	/**
	 * @return true to abort the transfer
	 */
	public boolean getAbort();
}
