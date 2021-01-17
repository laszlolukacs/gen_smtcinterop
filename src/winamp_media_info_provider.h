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

	[[nodiscard]] wchar_t* wa_ipc_get_current_filename() const;
	int wa_ipc_get_file_metadata(
		const wchar_t* filename,
		const wchar_t* metadata_to_query,
		wchar_t* metadata_out,
		unsigned int metadata_length);

public:
	static WinampMediaInfoProvider& get_instance()
	{
		static WinampMediaInfoProvider instance;
		return instance;
	}

	WinampMediaInfoProvider(WinampMediaInfoProvider const&) = delete;
	void operator=(WinampMediaInfoProvider const&) = delete;

	void set_window(HWND window);
	[[nodiscard]] MediaInfo* get_metadata_of_current_song() override;
};

#endif // WINAMP_MEDIA_INFO_PROVIDER_H
