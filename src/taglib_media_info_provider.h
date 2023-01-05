/* See LICENSE for copyright details. */

#ifndef TAGLIB_MEDIA_INFO_PROVIDER_H
#define TAGLIB_MEDIA_INFO_PROVIDER_H

#include "media_info_provider.h"

#include <taglib/fileref.h>
#include <taglib/apetag.h>
#include <taglib/asffile.h>
#include <taglib/id3v2tag.h>
#include <taglib/flacfile.h>
#include <taglib/mp4file.h>

#include "album_art.h"

class TaglibMediaInfoProvider : public MediaInfoProvider
{
	TaglibMediaInfoProvider() { }

	std::vector<unsigned char> convert_to_uchar(const char* char_array, const unsigned int size);
	AlbumArt extract_album_art_from_apev2(TagLib::APE::Tag* tag);
	AlbumArt extract_album_art_from_asf(TagLib::ASF::File* file);
	AlbumArt extract_album_art_from_id3v2(TagLib::ID3v2::Tag* tag);
	AlbumArt extract_album_art_from_flac(TagLib::FLAC::File* file);
	AlbumArt extract_album_art_from_mp4(TagLib::MP4::File* file);
	AlbumArt extract_album_art(const TagLib::FileRef& file);

public:
	static TaglibMediaInfoProvider& get_instance()
	{
		static TaglibMediaInfoProvider instance;
		return instance;
	}

	TaglibMediaInfoProvider(TaglibMediaInfoProvider const&) = delete;
	void operator=(TaglibMediaInfoProvider const&) = delete;

	[[nodiscard]] MediaInfo get_metadata_of_song(std::wstring filename) override;
};

#endif // TAGLIB_MEDIA_INFO_PROVIDER_H
