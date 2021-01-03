/* See LICENSE for copyright details. */

#include "gen_smtc.h"
#include "windows_smtc_wrapper.h"

winampGeneralPurposePlugin plugin = {
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

ComPtr<ISMTC> mControls;
ComPtr<ISMTCDisplayUpdater> mDisplay;

// event functions follow

int init() {
	//A basic messagebox that tells you the 'init' event has been triggered.
//If everything works you should see this message when you start Winamp once your plugin has been installed.
//You can change this later to do whatever you want (including nothing)
	MessageBox(plugin.hwndParent, L"Init event triggered for gen_smtc. Plugin installed successfully!", L"", MB_OK);
	g_lpOriginalWindowProc = (WNDPROC)(SetWindowLongPtr(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)WindowProc));
	Create(plugin.hwndParent);
	return 0;
}

void config() {
	//A basic messagebox that tells you the 'config' event has been triggered.
	//You can change this later to do whatever you want (including nothing)
	MessageBox(plugin.hwndParent, L"Config event triggered for gen_smtc.", L"", MB_OK);
}

void quit() {
	//A basic messagebox that tells you the 'quit' event has been triggered.
	//If everything works you should see this message when you quit Winamp once your plugin has been installed.
	//You can change this later to do whatever you want (including nothing)
	SetWindowLongPtr(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)g_lpOriginalWindowProc);
	MessageBox(0, L"Quit event triggered for gen_smtc.", L"", MB_OK);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_WA_IPC)
	{
		if (lParam == IPC_CB_MISC && wParam == IPC_CB_MISC_TITLE)
		{
			// song change
			if (mControls != nullptr)
			{
				HRESULT hr = mControls->put_PlaybackStatus(
					ABI::Windows::Media::MediaPlaybackStatus_Playing);

				if (FAILED(hr)) {
					MessageBox(plugin.hwndParent, L"Failure", L"", MB_OK);
					return hr;
				}

				ComPtr<ABI::Windows::Media::IMusicDisplayProperties> musicProps;

				hr = mDisplay->put_Type(ABI::Windows::Media::MediaPlaybackType::MediaPlaybackType_Music);
				hr = mDisplay->get_MusicProperties(musicProps.GetAddressOf());
				if (FAILED(hr)) {
					MessageBox(plugin.hwndParent, L"Failure", L"", MB_OK);
					return hr;
				}

				hr = musicProps->put_Artist(Microsoft::WRL::Wrappers::HStringReference(L"TEST ARTIST (WINAMP)").Get());
				if (FAILED(hr)) {
					MessageBox(plugin.hwndParent, L"Failure", L"", MB_OK);
					return hr;
				}

				hr = musicProps->put_Title(Microsoft::WRL::Wrappers::HStringReference(L"Test Title (WINAMP)").Get());
				if (FAILED(hr)) {
					MessageBox(plugin.hwndParent, L"Failure", L"", MB_OK);
					return hr;
				}

				hr = musicProps->put_AlbumArtist(Microsoft::WRL::Wrappers::HStringReference(L"Sample Album Artist (WINAMP)").Get());
				if (FAILED(hr)) {
					MessageBox(plugin.hwndParent, L"Failure", L"", MB_OK);
					return hr;
				}

				hr = mDisplay->Update();
				if (FAILED(hr)) {
					MessageBox(plugin.hwndParent, L"Failure", L"", MB_OK);
					return hr;
				}
			}

			MessageBox(plugin.hwndParent, L"Song Changed", L"", MB_OK);
		}
	}

	return CallWindowProc(g_lpOriginalWindowProc, hwnd, uMsg, wParam, lParam);
}

// This is an export function called by Winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() {
	return &plugin;
}
