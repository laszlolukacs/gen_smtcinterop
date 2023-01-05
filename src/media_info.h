/* See LICENSE for copyright details. */

#ifndef MEDIA_INFO_H
#define MEDIA_INFO_H

#include <string>
#include <utility>
#include "album_art.h"

class MediaInfo
{
private:
	std::wstring artist;
	std::wstring title;
	AlbumArt album_art;

public:
	MediaInfo(std::wstring artist, std::wstring title, AlbumArt album_art)
	{
		this->artist = std::move(artist);
		this->title = std::move(title);
		this->album_art = album_art;
	}

	MediaInfo(std::wstring artist, std::wstring title) : MediaInfo(artist, title, AlbumArt())
	{
	}

	std::wstring get_artist() const
	{
		return this->artist;
	}

	std::wstring get_title() const
	{
		return this->title;
	}

	AlbumArt get_album_art() const
	{
		return this->album_art;
	}

	bool empty() const
	{
		return title.empty()
			&& artist.empty()
			&& album_art.empty();
	}
};

#endif // MEDIA_INFO_H
