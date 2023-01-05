/* See LICENSE for copyright details. */

#ifndef ALBUM_ART_H
#define ALBUM_ART_H

#include <string>
#include <utility>
#include <vector>

class AlbumArt
{
private:
	std::vector<unsigned char> pixel_data;
	std::wstring mime;

public:
	AlbumArt()
	{
		this->pixel_data = std::vector<unsigned char>();
		this->mime = L"";
	}

	AlbumArt(std::vector<unsigned char> pixel_data, std::wstring mime)
	{
		this->pixel_data = std::move(pixel_data);
		this->mime = std::move(mime);
	}

	std::vector<unsigned char> get_pixel_data() const
	{
		return this->pixel_data;
	}

	std::wstring get_mime() const
	{
		return this->mime;
	}

	bool empty() const
	{
		return pixel_data.empty() || mime.empty();
	}
};

#endif // ALBUM_ART_H
