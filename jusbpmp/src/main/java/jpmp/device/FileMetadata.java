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

package jpmp.device;

/**
 * MTP track metadata implementation
 * 
 * @author cmi
 * <p>Notes : Not all MTP metadata properties are currently implemented</p>
 * <p>
 * Some MTP not implemented properties : date, duration, usecount etc.
 * See MTP specification for all metadata properties.
 * </p> 
 *
 */

public class FileMetadata {

	String title;
	String artist;
	String album;
	String genre;
	int 	rating;
	public String getTitle() {
		return title;
	}
	public void setTitle(String title) {
		this.title = title;
	}
	public String getArtist() {
		return artist;
	}
	public void setArtist(String artist) {
		this.artist = artist;
	}
	public String getAlbum() {
		return album;
	}
	public void setAlbum(String album) {
		this.album = album;
	}
	public String getGenre() {
		return genre;
	}
	public void setGenre(String genre) {
		this.genre = genre;
	}
	public int getRating() {
		return rating;
	}
	public void setRating(int rating) {
		this.rating = rating;
	}
	
	public String dump(){
		StringBuffer sb = new StringBuffer();
		sb.append("[title = " + title);
		sb.append("; album = " + album);
		sb.append("; artist = " + artist);
		sb.append("; genre = " + genre);
		sb.append("; rating = " + rating);
		sb.append("]");
		return sb.toString();
	}
}
