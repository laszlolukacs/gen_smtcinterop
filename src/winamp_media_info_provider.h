/* See LICENSE for copyright details. */

#ifndef WINAMP_MEDIA_INFO_PROVIDER_H
#define WINAMP_MEDIA_INFO_PROVIDER_H

#include <windows.h>

#include "media_info.h"
#include "media_info_provider.h"

class WinampMediaInfoProvider : public MediaInfoProvider
{
private:
	WinampMediaInfoProvider() { }

	HWND window = nullptr;

	[[nodiscard]]
	MediaInfo get_specified_metadata_of_file(
		const wchar_t* filename,
		const wchar_t* metadata_artist,
		const wchar_t* metadata_title);

public:
	static WinampMediaInfoProvider& get_instance()
	{
		static WinampMediaInfoProvider instance;
		return instance;
	}

	WinampMediaInfoProvider(WinampMediaInfoProvider const&) = delete;
	void operator=(WinampMediaInfoProvider const&) = delete;

	void set_window(HWND window);
	[[nodiscard]] MediaInfo get_metadata_of_song(std::wstring filename) override;
};

#endif // WINAMP_MEDIA_INFO_PROVIDER_H
