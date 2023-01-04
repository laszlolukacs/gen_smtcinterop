#include "taglib_media_info_provider.h"

#include <taglib/fileref.h>

MediaInfo TaglibMediaInfoProvider::get_metadata_of_song(std::wstring filename)
{
	if (!filename.empty()) {
		const TagLib::FileRef f(filename.c_str());
		if (f.tag() != nullptr)
		{
			TagLib::String artist = f.tag()->artist();
			TagLib::String title = f.tag()->title();
			return MediaInfo(artist.toWString(), title.toWString());
		}
	}

	return MediaInfo(L"", L"");
}
