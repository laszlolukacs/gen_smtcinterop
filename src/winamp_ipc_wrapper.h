/* See LICENSE for copyright details. */

#ifndef WINAMP_IPC_WRAPPER_H
#define WINAMP_IPC_WRAPPER_H

#include <string>

#include <windows.h>
#include <wa_ipc.h>

#define WINAMP_METADATA_ARTIST L"artist"
#define WINAMP_METADATA_TITLE L"title"

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

};

#endif // WINAMP_IPC_WRAPPER_H
