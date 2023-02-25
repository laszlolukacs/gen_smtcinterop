/* See LICENSE for copyright details. */

#ifndef WINAMP_IPC_WRAPPER_H
#define WINAMP_IPC_WRAPPER_H

#include <string>

#include <windows.h>
#include <wa_ipc.h>

#include "playback_state.h"

struct WinampIpcWrapper
{
	static HWND hwnd;

	[[nodiscard]] static std::wstring wa_ipc_get_current_filename()
	{
		const int current_playlist_index =
			SendMessage(
				hwnd,
				WM_WA_IPC,
				0,
				IPC_GETLISTPOS);

		auto* const filename =
			(wchar_t*)SendMessage(
				hwnd,
				WM_WA_IPC,
				current_playlist_index,
				IPC_GETPLAYLISTFILEW);

		return std::wstring(filename);
	}

	[[nodiscard]] static std::wstring wa_ipc_get_current_title()
	{
		const int current_playlist_index =
			SendMessage(
				hwnd,
				WM_WA_IPC,
				0,
				IPC_GETLISTPOS);

		auto* const track_title =
			(wchar_t*)SendMessage(
				hwnd,
				WM_WA_IPC,
				current_playlist_index,
				IPC_GETPLAYLISTTITLEW);

		return std::wstring(track_title);
	}

	static int wa_ipc_get_file_metadata(
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
				hwnd,
				WM_WA_IPC,
				(WPARAM)&file_metadata_field,
				IPC_GET_EXTENDED_FILE_INFOW);
		return is_ipc_call_supported;
	}

	static PlaybackState wa_ipc_is_playing()
	{
		const int is_wa_playing = SendMessage(
			hwnd,
			WM_WA_IPC,
			0,
			IPC_ISPLAYING);
		switch (is_wa_playing) {
		case 1:
			return PlaybackState::playing;
		case 3:
			return PlaybackState::paused;
		case 0:
			return PlaybackState::stopped;
		default:
			return PlaybackState::none;
		}
	}
};

#endif // WINAMP_IPC_WRAPPER_H
