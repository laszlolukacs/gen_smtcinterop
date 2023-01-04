/* See LICENSE for copyright details. */

#ifndef TAGLIB_MEDIA_INFO_PROVIDER_H
#define TAGLIB_MEDIA_INFO_PROVIDER_H

#include "media_info_provider.h"

class TaglibMediaInfoProvider : public MediaInfoProvider
{
	TaglibMediaInfoProvider() { }

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
