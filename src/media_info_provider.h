/* See LICENSE for copyright details. */

#ifndef MEDIA_INFO_PROVIDER_H
#define MEDIA_INFO_PROVIDER_H

#include "media_info.h"

struct MediaInfoProvider
{
	virtual MediaInfo* get_metadata_of_current_song() = 0;

	virtual ~MediaInfoProvider() = default;
};

#endif // MEDIA_INFO_PROVIDER_H
