/* See LICENSE for copyright details. */

#include "winamp_media_info_provider.h"

#include <cassert>
#include "winamp_ipc_wrapper.h"


void WinampMediaInfoProvider::set_window(HWND window)
{
	this->window = window;
}

MediaInfo WinampMediaInfoProvider::get_metadata_of_song(std::wstring filename)
{
	const wchar_t* current_filename = filename.c_str();
	if (current_filename != nullptr)
	{
		wchar_t artist_name[256];
		const bool received_artist = WinampIpcWrapper::wa_ipc_get_file_metadata(
			current_filename,
			WINAMP_METADATA_ARTIST,
			artist_name,
			256) && artist_name[0];

		wchar_t song_title[256];
		const bool received_title = WinampIpcWrapper::wa_ipc_get_file_metadata(
			current_filename,
			WINAMP_METADATA_TITLE,
			song_title,
			256) && song_title[0];

		if (received_title)
		{
			if (!received_artist)
			{
				return MediaInfo(L"", &song_title[0]);
			}

			return MediaInfo(&artist_name[0], &song_title[0]);
		}
	}

	return MediaInfo(L"", L"");
}
