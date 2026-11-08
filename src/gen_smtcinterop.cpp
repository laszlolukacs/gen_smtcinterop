/* See LICENSE for copyright details. */

#include "gen_smtcinterop.h"

#ifdef _DEBUG
#include <cassert>
#endif

#include <wa_ipc.h>

#include <unordered_map>

#include "debug_log.h"
#include "media_key_throttle.h"
#include "winamp_ipc_wrapper.h"
#include "winamp_media_info_provider.h"
#include "taglib_media_info_provider.h"
#include "winamp_playback_wrapper.h"
#include "windows_resources_wrapper.h"
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

bool initialized = false;

HWND WinampIpcWrapper::hwnd = nullptr;

void on_song_change();

LRESULT CALLBACK WindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

namespace
{
	// Winamp handles multimedia keys in all of its top-level windows (player,
	// playlist editor, equalizer, media library, video). Subclassing only the
	// main window means that a media key delivered to any other Winamp window
	// is handled by Winamp itself while DefWindowProc still forwards it to the
	// shell, which routes it to SMTC as well. The result is two actions per key
	// press. Subclass all Winamp windows so that exactly one action is
	// performed, no matter which window receives the key.
	std::unordered_map<HWND, WNDPROC> g_original_window_procs;

	DWORD g_gui_thread_id = 0;

	const UINT_PTR window_rescan_timer_id = 0x534D5443; // 'SMTC'
	const UINT window_rescan_interval_ms = 1000;

	bool has_winamp_window_class(HWND hwnd)
	{
		wchar_t class_name[64] = {};
		if (GetClassNameW(hwnd, class_name, _countof(class_name)) == 0)
		{
			return false;
		}

		return _wcsnicmp(class_name, L"Winamp", 6) == 0;
	}

	bool is_winamp_window(HWND hwnd)
	{
		if (hwnd == nullptr || !IsWindow(hwnd))
		{
			return false;
		}

		DWORD process_id = 0;
		const DWORD window_thread_id = GetWindowThreadProcessId(hwnd, &process_id);
		if (process_id != GetCurrentProcessId() || window_thread_id != g_gui_thread_id)
		{
			// A window procedure may only be changed by the thread that created
			// the window. Winamp has helper windows on other threads; leaving
			// those alone is required for stability.
			return false;
		}

		// Only top-level windows have their own window procedure that can
		// receive an application command. Winamp's player, playlist, equalizer,
		// media library and video windows are top-level windows, but some of
		// them are owned by the player window, so do not filter by owner.
		const LONG_PTR window_style = GetWindowLongPtrW(hwnd, GWL_STYLE);
		if ((window_style & WS_CHILD) != 0)
		{
			return false;
		}

		return has_winamp_window_class(hwnd);
	}

	void subclass_winamp_window(HWND hwnd)
	{
		if (!is_winamp_window(hwnd))
		{
			return;
		}

		if (g_original_window_procs.find(hwnd) != g_original_window_procs.end())
		{
			// Already subclassed. Never re-subclass a window, even if its
			// window procedure no longer is ours: Winamp may have chained
			// its own procedure on top of ours (still calling ours), and
			// subclassing again would then create a WindowProc <-> Winamp
			// procedure loop that crashes inside USER32.
			return;
		}

		WNDPROC const original_window_proc =
			(WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
		if (original_window_proc != nullptr && original_window_proc != WindowProc)
		{
			g_original_window_procs.emplace(hwnd, original_window_proc);
			debug_log(L"subclassed window hwnd=%p", hwnd);
		}
	}

	BOOL CALLBACK subclass_winamp_window_callback(HWND hwnd, LPARAM)
	{
		subclass_winamp_window(hwnd);
		return TRUE;
	}

	void subclass_winamp_windows()
	{
		// Forget windows that no longer exist, so that a window handle that
		// gets reused by a new Winamp window can be subclassed again.
		for (auto it = g_original_window_procs.begin(); it != g_original_window_procs.end();)
		{
			if (!IsWindow(it->first))
			{
				it = g_original_window_procs.erase(it);
			}
			else
			{
				++it;
			}
		}

		EnumWindows(subclass_winamp_window_callback, 0);
	}

	void unsubclass_winamp_windows()
	{
		for (auto const& entry : g_original_window_procs)
		{
			// Only restore the window procedure if ours is still installed.
			// If Winamp chained its own procedure on top, leave the chain
			// intact; ripping our procedure out of the middle would break it.
			if (IsWindow(entry.first) &&
				(WNDPROC)GetWindowLongPtrW(entry.first, GWLP_WNDPROC) == WindowProc)
			{
				SetWindowLongPtrW(entry.first, GWLP_WNDPROC, (LONG_PTR)entry.second);
			}
		}
		g_original_window_procs.clear();
	}

	void install_window_hooks()
	{
		g_gui_thread_id = GetWindowThreadProcessId(plugin.hwndParent, nullptr);
		subclass_winamp_windows();

		// Re-scan periodically to subclass windows that are created later
		// (e.g. when the user opens the playlist editor or the equalizer).
		const UINT_PTR rescan_timer = SetTimer(
			plugin.hwndParent,
			window_rescan_timer_id,
			window_rescan_interval_ms,
			nullptr);

		debug_log(
			L"install_window_hooks: gui_thread_id=%lu rescan_timer=%llu subclassed_windows=%zu",
			g_gui_thread_id,
			(unsigned long long)rescan_timer,
			g_original_window_procs.size());
	}

	void remove_window_hooks()
	{
		KillTimer(plugin.hwndParent, window_rescan_timer_id);
		unsubclass_winamp_windows();
	}
}

int init()
{
#ifdef _DEBUG
	assert(plugin.hwndParent != nullptr);
#endif
	debug_log(L"init: hwnd=%p", plugin.hwndParent);
	initialized = WindowsSystemMediaTransportControlsWrapper::get_instance().initialize(plugin.hwndParent);
	debug_log(L"init: initialized=%d smtc_enabled=%d", initialized, WindowsSystemMediaTransportControlsWrapper::get_instance().is_enabled());
	if (!initialized)
	{
#ifdef _DEBUG
		std::wstring debug_message = L"init() event for gen_smtcinterop did not succeed, unable to initialize System Media Transport Controls.\n";
		OutputDebugString(debug_message.c_str());
#endif
		return 0;
	}

	WinampIpcWrapper::hwnd = plugin.hwndParent;
	WinampPlaybackWrapper::get_instance().set_window(plugin.hwndParent);
	WinampMediaInfoProvider::get_instance().set_window(plugin.hwndParent);

	on_song_change();

	install_window_hooks();
#ifdef _DEBUG
	// If everything works you should see this message when you start Winamp once your plugin has been installed.
	std::wstring debug_message = L"init() event for gen_smtcinterop finished. Plugin installed successfully!\n";
	OutputDebugString(debug_message.c_str());
#endif

	return 0;
}

void config()
{
	WindowsResourcesWrapper::show_about_taskdialog(plugin.hDllInstance);
}

void quit()
{
	if (!initialized)
	{
		return;
	}

	debug_log(L"quit");
	WindowsSystemMediaTransportControlsWrapper::get_instance().clear_metadata();
	WinampIpcWrapper::hwnd = nullptr;

	// restore the original window procedures when shutting down
	remove_window_hooks();

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
	debug_log(L"on_song_change: filename=\"%ls\" metadata_empty=%d", current_filename.c_str(), media_info.empty() ? 1 : 0);
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
			smtc.set_thumbnail(WindowsResourcesWrapper::get_default_album_art((LPCTSTR)WindowProc));
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
	if (uMsg == WM_WA_IPC && hwnd == plugin.hwndParent)
	{
		if (lParam == IPC_CB_MISC && initialized)
		{
			if (wParam == IPC_CB_MISC_TITLE)
			{
				// whenever the currently played song changes in Winamp, the SMTC display will be updated
				debug_log(L"WndProc: IPC_CB_MISC_TITLE");
				on_song_change();
			}
			else if (wParam == IPC_CB_MISC_STATUS)
			{
				const PlaybackState state = WinampIpcWrapper::wa_ipc_is_playing();
				debug_log(L"WndProc: IPC_CB_MISC_STATUS state=%d", (int)state);
				WindowsSystemMediaTransportControlsWrapper::get_instance().set_playback_status(state);
			}
		}
	}

	if (uMsg == WM_TIMER && wParam == window_rescan_timer_id)
	{
		subclass_winamp_windows();
		return 0;
	}

	if (uMsg == WM_APPCOMMAND)
	{
		const WORD appcommand = GET_APPCOMMAND_LPARAM(lParam);
		debug_log(
			L"WndProc: WM_APPCOMMAND hwnd=%p appcommand=%u lParam=%p wParam=%p initialized=%d",
			hwnd,
			(unsigned)appcommand,
			(void*)lParam,
			(void*)wParam,
			initialized ? 1 : 0);

		if (initialized)
		{
			// Winamp processes multimedia keys in every one of its top-level
			// windows and then lets DefWindowProc forward the message to the
			// shell, which routes it to System Media Transport Controls as
			// well. That would perform two actions per key press (e.g. play
			// followed by pause). Handle the transport commands here and
			// report them as handled so that exactly one action is performed.
			const media_key_action::type action = media_key_action::from_appcommand(appcommand);
			if (!media_key_action_allowed(action))
			{
				// The same physical key press was already handled moments ago
				// (e.g. via the SMTC button pressed callback). Swallow it.
				debug_log(L"WndProc: WM_APPCOMMAND appcommand=%u deduplicated", (unsigned)appcommand);
				return TRUE;
			}

			WinampPlaybackWrapper& playback_controls = WinampPlaybackWrapper::get_instance();
			bool handled = true;
			switch (appcommand)
			{
			case APPCOMMAND_MEDIA_PLAY:
				playback_controls.play();
				break;
			case APPCOMMAND_MEDIA_PAUSE:
				playback_controls.pause();
				break;
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
				if (WinampIpcWrapper::wa_ipc_is_playing() == PlaybackState::playing)
				{
					playback_controls.pause();
				}
				else
				{
					playback_controls.play();
				}
				break;
			case APPCOMMAND_MEDIA_STOP:
				playback_controls.stop();
				break;
			case APPCOMMAND_MEDIA_NEXTTRACK:
				playback_controls.next_track();
				break;
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
				playback_controls.previous_track();
				break;
			case APPCOMMAND_MEDIA_FAST_FORWARD:
				SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_FFWD5S, 0);
				break;
			case APPCOMMAND_MEDIA_REWIND:
				SendMessage(plugin.hwndParent, WM_COMMAND, WINAMP_REW5S, 0);
				break;
			default:
				handled = false;
				break;
			}

			if (handled)
			{
				debug_log(L"WndProc: WM_APPCOMMAND appcommand=%u handled", (unsigned)appcommand);
				return TRUE;
			}
		}
	}

	auto const original_window_proc = g_original_window_procs.find(hwnd);
	if (original_window_proc != g_original_window_procs.end())
	{
		return CallWindowProc(original_window_proc->second, hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// This is an export function called by Winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() {
	return &plugin;
}
