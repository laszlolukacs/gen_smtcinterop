/* See LICENSE for copyright details. */

#include "gen_smtc.h"

#ifdef _DEBUG
#include <cassert>
#endif

#include <CommCtrl.h>
#include <VersionHelpers.h>

#include <wa_ipc.h>

#include "winamp_ipc_wrapper.h"
#include "winamp_media_info_provider.h"
#include "taglib_media_info_provider.h"
#include "winamp_playback_wrapper.h"
#include "windows_smtc_wrapper.h"

#pragma comment(lib, "ComCtl32.lib")

winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
	PLUGIN_NAME,
	init,
	config,
	quit,
	0,
	0
};

bool initialized = false;

HWND WinampIpcWrapper::hwnd = nullptr;

void on_song_change();

WNDPROC g_lpOriginalWindowProc = 0;

LRESULT CALLBACK WindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

int init()
{
	// System Media Transport Controls are available in Windows 8, but can be used from desktop applications only starting with Windows 10
	if (!IsWindows10OrGreater())
	{
#ifdef _DEBUG
		std::wstring older_than_windows_10_message = L"This plugin requires at least Windows 10 to work properly.\n";
		OutputDebugString(older_than_windows_10_message.c_str());
#endif
		return 0;
	}

#ifdef _DEBUG
	assert(plugin.hwndParent != nullptr);
#endif
	initialized = WindowsSystemMediaTransportControlsWrapper::get_instance().initialize(plugin.hwndParent);
	if (!initialized)
	{
#ifdef _DEBUG
		std::wstring debug_message = L"init() event for gen_smtc did not succeed, unable to initialize System Media Transport Controls.\n";
		OutputDebugString(debug_message.c_str());
#endif
		return 0;
	}

	WinampIpcWrapper::hwnd = plugin.hwndParent;
	WinampPlaybackWrapper::get_instance().set_window(plugin.hwndParent);
	WinampMediaInfoProvider::get_instance().set_window(plugin.hwndParent);

	on_song_change();

	// replaces the WndProc of the parent window with the one defined in this class
	g_lpOriginalWindowProc = (WNDPROC)(SetWindowLongPtr(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)WindowProc));
#ifdef _DEBUG
	// If everything works you should see this message when you start Winamp once your plugin has been installed.
	std::wstring debug_message = L"init() event for gen_smtc finished. Plugin installed successfully!\n";
	OutputDebugString(debug_message.c_str());
#endif

	return 0;
}

void config()
{
	const wchar_t* about_text = L"Version 1.0\nCopyright (C) 2020-2023 Laszlo Lukacs\n\n\
		This program is free software; you can redistribute it and/or modify\n\
		it under the terms of the GNU General Public License as published by\n\
		the Free Software Foundation; either version 2 of the License, or\n\
		(at your option) any later version.\n\n\
		This program is distributed in the hope that it will be useful,\n\
		but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
		GNU General Public License for more details.\n\n\
		You should have received a copy of the GNU General Public License along\n\
		with this program; if not, write to the Free Software Foundation, Inc.,\n\
		51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.";

	int nButtonPressed = 0;
	TASKDIALOGCONFIG TaskDialogConfig = { 0 };
	TaskDialogConfig.cbSize = sizeof(TASKDIALOGCONFIG);
	TaskDialogConfig.hwndParent = NULL;
	TaskDialogConfig.hInstance = plugin.hDllInstance;
	TaskDialogConfig.dwFlags = TDF_SIZE_TO_CONTENT;
	TaskDialogConfig.dwCommonButtons = TDCBF_OK_BUTTON;
	TaskDialogConfig.pszWindowTitle = L"About";
	TaskDialogConfig.pszMainInstruction = L"System Media Transport Controls Integration Plug-in";
	TaskDialogConfig.pszContent = about_text;
	if (!IsWindows10OrGreater())
	{
		TaskDialogConfig.pszFooter = L"This plugin requires at least Windows 10 to work properly. It will not function on your system.";
		TaskDialogConfig.pszFooterIcon = TD_ERROR_ICON;
	}

	TaskDialogIndirect(&TaskDialogConfig, &nButtonPressed, NULL, NULL);
}

void quit()
{
	if (!IsWindows10OrGreater() || !initialized)
	{
		return;
	}

	WindowsSystemMediaTransportControlsWrapper::get_instance().clear_metadata();
	WinampIpcWrapper::hwnd = nullptr;

	// restores the original WndProc of the parent window when shutting down
	SetWindowLongPtr(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)g_lpOriginalWindowProc);

#ifdef _DEBUG
	if (_CrtDumpMemoryLeaks())
	{
		_ASSERTE(_CrtCheckMemory());
	}
#endif
}

void on_song_change()
{
	auto const current_filename = WinampIpcWrapper::wa_ipc_get_current_filename();
	MediaInfoProvider& media_info_provider = TaglibMediaInfoProvider::get_instance();
	auto media_info = media_info_provider.get_metadata_of_song(current_filename);
	if (media_info.empty())
	{
		// fall back to Winamp API when TagLib cannot extract metadata (e.g. in case of online streams)
		MediaInfoProvider& media_info_provider = WinampMediaInfoProvider::get_instance();
		media_info = media_info_provider.get_metadata_of_song(current_filename);
	}

	SystemMediaControls& smtc = WindowsSystemMediaTransportControlsWrapper::get_instance();
	if (!media_info.empty())
	{
		smtc.set_artist_and_track(media_info.get_artist(), media_info.get_title());
		if (!media_info.empty() && !media_info.get_album_art().empty())
		{
			smtc.set_thumbnail(media_info.get_album_art().get_pixel_data());
		}
		else
		{
			smtc.clear_thumbnail();
		}
	}
	else
	{
		smtc.clear_metadata();
	}
}

LRESULT CALLBACK WindowProc(
	_In_ HWND hwnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	if (uMsg == WM_WA_IPC)
	{
		if (lParam == IPC_CB_MISC && initialized)
		{
			if (wParam == IPC_CB_MISC_TITLE)
			{
				// whenever the currently played song changes in Winamp, the SMTC display will be updated
				on_song_change();
			}
			else if (wParam == IPC_CB_MISC_STATUS)
			{
				WindowsSystemMediaTransportControlsWrapper::get_instance().set_playback_status(WinampIpcWrapper::wa_ipc_is_playing());
			}
		}
	}

	return CallWindowProc(g_lpOriginalWindowProc, hwnd, uMsg, wParam, lParam);
}

// This is an export function called by Winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() {
	return &plugin;
}
