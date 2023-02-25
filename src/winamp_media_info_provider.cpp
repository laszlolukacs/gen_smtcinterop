/* See LICENSE for copyright details. */

#include "winamp_media_info_provider.h"

#include "winamp_ipc_wrapper.h"

// list of valid Winamp API ExtendedFileInfoW metadata keys: http://forums.shoutcast.com/showpost.php?p=3226776&postcount=5
#define WINAMP_METADATA_ARTIST L"artist"
#define WINAMP_METADATA_TITLE L"title"
#define WINAMP_METADATA_STREAM_NAME L"streamname"
#define WINAMP_METADATA_STREAM_TITLE L"streamtitle"

void WinampMediaInfoProvider::set_window(HWND window)
{
	this->window = window;
}

MediaInfo WinampMediaInfoProvider::get_metadata_of_song(std::wstring filename)
{
	const wchar_t* current_filename = filename.c_str();
	if (current_filename != nullptr)
	{
		MediaInfo media_info = get_specified_metadata_of_file(current_filename, WINAMP_METADATA_ARTIST, WINAMP_METADATA_TITLE);
		if (media_info.empty())
		{
			// loading stream media info: http://forums.shoutcast.com/showthread.php?t=458950
			media_info = get_specified_metadata_of_file(current_filename, WINAMP_METADATA_STREAM_NAME, WINAMP_METADATA_STREAM_TITLE);
		}

		if (media_info.empty())
		{
			const auto track_title = WinampIpcWrapper::wa_ipc_get_current_title();
			return MediaInfo(L"", track_title);
		}

		return media_info;
	}

	return MediaInfo(L"", L"");
}

MediaInfo WinampMediaInfoProvider::get_specified_metadata_of_file(
	const wchar_t* current_filename,
	const wchar_t* metadata_for_artist_slot,
	const wchar_t* metadata_for_title_slot)
{
	wchar_t artist_name[256];
	const bool received_artist = WinampIpcWrapper::wa_ipc_get_file_metadata(
		current_filename,
		metadata_for_artist_slot,
		artist_name,
		256) && artist_name[0];

	wchar_t song_title[256];
	const bool received_title = WinampIpcWrapper::wa_ipc_get_file_metadata(
		current_filename,
		metadata_for_title_slot,
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

	return MediaInfo(L"", L"");
}
