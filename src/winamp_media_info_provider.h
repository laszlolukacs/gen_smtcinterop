/* See LICENSE for copyright details. */

#ifndef WINAMP_MEDIA_INFO_PROVIDER_H
#define WINAMP_MEDIA_INFO_PROVIDER_H

#include <windows.h>

#include "winamp_wasabi.h"
#include <AlbumArt/svc_albumArtProvider.h>

#include "media_info.h"
#include "media_info_provider.h"

class WinampMediaInfoProvider : public MediaInfoProvider
{
private:
	WinampMediaInfoProvider() { }

	HWND window = nullptr;
	api_memmgr* wasabi_memory_manager = nullptr;
	svc_albumArtProvider* album_art_provider = nullptr;

	unsigned int* wasabi_get_album_art(const wchar_t* filename) const;

public:
	static WinampMediaInfoProvider& get_instance()
	{
		static WinampMediaInfoProvider instance;
		return instance;
	}

	WinampMediaInfoProvider(WinampMediaInfoProvider const&) = delete;
	void operator=(WinampMediaInfoProvider const&) = delete;

	void set_window(HWND window);
	void init_winamp_services();
	void deregister_winamp_services();
	[[nodiscard]] MediaInfo get_metadata_of_song(std::wstring filename) override;
};

#endif // WINAMP_MEDIA_INFO_PROVIDER_H
