/* See LICENSE for copyright details. */

#include "gen_smtc.h"

#ifdef _DEBUG
#include <cassert>
#endif
#include <wa_ipc.h>

#include "winamp_media_info_provider.h"
#include "winamp_playback_wrapper.h"
#include "windows_smtc_wrapper.h"

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

WNDPROC g_lpOriginalWindowProc = 0;

LRESULT CALLBACK WindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

SystemMediaControls& g_smtc = WindowsSystemMediaTransportControlsWrapper::get_instance();

int init()
{
#ifdef _DEBUG
	assert(plugin.hwndParent != nullptr);
#endif
	WinampPlaybackWrapper::get_instance().set_window(plugin.hwndParent);
	WinampMediaInfoProvider::get_instance().set_window(plugin.hwndParent);
	g_smtc.initialize(plugin.hwndParent);

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
	MessageBox(plugin.hwndParent, L"Config event triggered for gen_smtc.", L"", MB_OK);
}

void quit()
{
	g_smtc.clear_metadata();

	// restores the original WndProc of the parent window when shutting down
	SetWindowLongPtr(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)g_lpOriginalWindowProc);

#ifdef _DEBUG
	if (_CrtDumpMemoryLeaks())
	{
		_ASSERTE(_CrtCheckMemory());
	}
#endif
}

LRESULT CALLBACK WindowProc(
	_In_ HWND hwnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	if (uMsg == WM_WA_IPC)
	{
		if (lParam == IPC_CB_MISC && wParam == IPC_CB_MISC_TITLE)
		{
			// whenever the currently played song changes in Winamp, the SMTC display will be updated
			MediaInfoProvider& media_info_provider = WinampMediaInfoProvider::get_instance();
			auto* const media_info = media_info_provider.get_metadata_of_current_song();
			if (media_info != nullptr)
			{
				g_smtc.set_artist_and_track(media_info->get_artist(), media_info->get_title());
			}
			else
			{
				g_smtc.set_artist_and_track(L"", L"");
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
