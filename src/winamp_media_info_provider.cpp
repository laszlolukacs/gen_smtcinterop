/* See LICENSE for copyright details. */

#include "winamp_media_info_provider.h"

#include <wa_ipc.h>

#define WINAMP_METADATA_ARTIST L"artist"
#define WINAMP_METADATA_TITLE L"title"

wchar_t* WinampMediaInfoProvider::wa_ipc_get_current_filename() const
{
	const int current_playlist_index =
		SendMessage(
			this->window,
			WM_WA_IPC,
			0,
			IPC_GETLISTPOS);

	auto* const filename =
		(wchar_t*)SendMessage(
			this->window,
			WM_WA_IPC,
			current_playlist_index,
			IPC_GETPLAYLISTFILEW);
	return filename;
}

int WinampMediaInfoProvider::wa_ipc_get_file_metadata(
	const wchar_t* filename,
	const wchar_t* metadata_to_query,
	wchar_t* metadata_out,
	const unsigned int metadata_length)
{
	metadata_out[0] = 0;
	extendedFileInfoStructW file_metadata_field =
	{
		filename,
		metadata_to_query,
		metadata_out,
		metadata_length
	};

	// returns 1 if the Winamp instance supports this IPC call
	auto const is_ipc_call_supported =
		(int)SendMessage(
			this->window,
			WM_WA_IPC,
			(WPARAM)&file_metadata_field,
			IPC_GET_EXTENDED_FILE_INFOW);
	return is_ipc_call_supported;
}

void WinampMediaInfoProvider::set_window(HWND window)
{
	this->window = window;
}

MediaInfo* WinampMediaInfoProvider::get_metadata_of_current_song()
{
	auto* const current_filename = this->wa_ipc_get_current_filename();
	if (current_filename != nullptr)
	{
		wchar_t artist_name[256];
		const bool received_artist = this->wa_ipc_get_file_metadata(
			current_filename,
			WINAMP_METADATA_ARTIST,
			artist_name,
			256) && artist_name[0];

		wchar_t song_title[256];
		const bool received_title = this->wa_ipc_get_file_metadata(
			current_filename,
			WINAMP_METADATA_TITLE,
			song_title,
			256) && song_title[0];

		if (received_title)
		{
			if (!received_artist)
			{
				return new MediaInfo(L"", &song_title[0]);
			}

			return new MediaInfo(&artist_name[0], &song_title[0]);
		}
	}

	return nullptr;
}
