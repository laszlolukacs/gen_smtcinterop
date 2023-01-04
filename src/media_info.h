/* See LICENSE for copyright details. */

#ifndef MEDIA_INFO_H
#define MEDIA_INFO_H

#include <string>
#include <utility>

class MediaInfo
{
private:
	std::wstring artist;
	std::wstring title;

public:
	MediaInfo(std::wstring artist, std::wstring title)
	{
		this->artist = std::move(artist);
		this->title = std::move(title);
	}

	std::wstring get_artist() const
	{
		return this->artist;
	}

	std::wstring get_title() const
	{
		return this->title;
	}

	~MediaInfo()
	{
	}
};

#endif // MEDIA_INFO_H
