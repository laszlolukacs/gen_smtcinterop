/* See LICENSE for copyright details. */

#include "windows_smtc_wrapper.h"

#ifdef _DEBUG
#include <cassert>
#endif
#include <SystemMediaTransportControlsInterop.h>
#include <wrl/event.h>
#include <shcore.h>
#include <Shlwapi.h>

#include "debug_log.h"
#include "media_key_throttle.h"
#include "playback_controls.h"
#include "winamp_ipc_wrapper.h"
#include "winamp_playback_wrapper.h"

#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "ShLwApi.lib")

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Media;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

bool WindowsSystemMediaTransportControlsWrapper::initialize(HWND hwnd)
{
	debug_log(L"smtc initialize: hwnd=%p", hwnd);
	if (hwnd == nullptr || !IsWindow(hwnd))
	{
		initialized = false;
		return false;
	}

	this->window = hwnd;
	ComPtr<ISystemMediaTransportControlsInterop> smtc_interop;
	HRESULT hr = GetActivationFactory(
		HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(),
		smtc_interop.GetAddressOf());
	if (FAILED(hr) || smtc_interop == nullptr)
	{
		initialized = false;
		return false;
	}

	hr = smtc_interop->GetForWindow((this->window), IID_PPV_ARGS((this->smtc).GetAddressOf()));
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	auto handler = Microsoft::WRL::Callback<ITypedEventHandler<
		SystemMediaTransportControls*,
		SystemMediaTransportControlsButtonPressedEventArgs*>>(
			&WindowsSystemMediaTransportControlsWrapper::button_pressed_callback);

	hr = (this->smtc)->add_ButtonPressed(handler.Get(), &(this->event_registration_token));
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	hr = (this->smtc)->put_IsEnabled(true);
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}
	this->smtc_enabled = true;

	hr = (this->smtc)->put_IsPlayEnabled(true);
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	hr = (this->smtc)->put_IsPauseEnabled(true);
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	hr = (this->smtc)->put_IsNextEnabled(true);
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	hr = (this->smtc)->put_IsPreviousEnabled(true);
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	hr = (this->smtc)->put_IsStopEnabled(true);
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	hr = (this->smtc)->
		put_PlaybackStatus(MediaPlaybackStatus::MediaPlaybackStatus_Stopped);
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	hr = (this->smtc)->get_DisplayUpdater((this->smtc_display_updater).GetAddressOf());
	if (FAILED(hr))
	{
		initialized = false;
		return false;
	}

	initialized = true;
	set_artist_and_track(L"Winamp", L"");
	debug_log(L"smtc initialize: done smtc_enabled=%d", this->smtc_enabled);

	return true;
}

bool WindowsSystemMediaTransportControlsWrapper::is_enabled() const
{
	return this->initialized && this->smtc_enabled;
}

bool WindowsSystemMediaTransportControlsWrapper::set_artist_and_track(std::wstring artist_name, std::wstring track_name)
{
	debug_log(L"set_artist_and_track: artist=\"%ls\" track=\"%ls\"", artist_name.c_str(), track_name.c_str());
	if (!initialized || this->smtc_display_updater == nullptr)
	{
		return false;
	}

	HRESULT hr = (this->smtc)->put_IsEnabled(true);
	if (FAILED(hr))
	{
		return false;
	}
	this->smtc_enabled = true;

	hr = (this->smtc_display_updater)->put_Type(MediaPlaybackType::MediaPlaybackType_Music);
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc_display_updater)->get_MusicProperties(&(this->smtc_music_display_properties));
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc_music_display_properties)->put_Artist(HStringReference(artist_name.c_str()).Get());
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc_music_display_properties)->put_Title(HStringReference(track_name.c_str()).Get());
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc_display_updater)->Update();
	return SUCCEEDED(hr);
}

bool WindowsSystemMediaTransportControlsWrapper::set_playback_status(PlaybackState playback_status)
{
	debug_log(L"set_playback_status: state=%d", (int)playback_status);
	if (!initialized
		|| this->smtc == nullptr
		|| this->smtc_display_updater == nullptr)
	{
		return false;
	}

	HRESULT hr;
	switch (playback_status) {
	case PlaybackState::stopped:
		hr = (this->smtc)->
			put_PlaybackStatus(MediaPlaybackStatus::MediaPlaybackStatus_Stopped);
		break;
	case PlaybackState::paused:
		hr = (this->smtc)->
			put_PlaybackStatus(MediaPlaybackStatus::MediaPlaybackStatus_Paused);
		break;
	case PlaybackState::playing:
		hr = (this->smtc)->
			put_PlaybackStatus(MediaPlaybackStatus::MediaPlaybackStatus_Playing);
		break;
	default:
		hr = (this->smtc)->
			put_PlaybackStatus(MediaPlaybackStatus::MediaPlaybackStatus_Stopped);
	}

	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc_display_updater)->Update();
	return SUCCEEDED(hr);
}

bool WindowsSystemMediaTransportControlsWrapper::set_thumbnail(std::vector<unsigned char> thumbnail_bytes)
{
	if (!initialized || this->smtc_display_updater == nullptr)
	{
		return false;
	}

	HRESULT hr;
	bool cover_loaded = false;
	if (thumbnail_bytes.size() > 0)
	{
		ComPtr<IStream> thumbnail_bytes_stream;
		thumbnail_bytes_stream.Attach(
			SHCreateMemStream((const BYTE*)thumbnail_bytes.data(), (UINT)thumbnail_bytes.size()));
		if (thumbnail_bytes_stream != nullptr)
		{
			ComPtr<IRandomAccessStream> thumbnail_pic_stream;
			hr = CreateRandomAccessStreamOverStream(
				thumbnail_bytes_stream.Get(),
				BSOS_DEFAULT,
				__uuidof(IRandomAccessStream),
				(void**)thumbnail_pic_stream.ReleaseAndGetAddressOf());
			if (FAILED(hr))
			{
				return false;
			}

			ComPtr<IRandomAccessStreamReferenceStatics>	reference_statics;
			hr = GetActivationFactory(
				HStringReference(RuntimeClass_Windows_Storage_Streams_RandomAccessStreamReference).Get(),
				reference_statics.GetAddressOf());
			if (FAILED(hr))
			{
				return false;
			}

			ComPtr<IRandomAccessStreamReference> thumbnail_pic_stream_reference;
			hr = reference_statics->CreateFromStream(thumbnail_pic_stream.Get(),
				thumbnail_pic_stream_reference.ReleaseAndGetAddressOf());
			if (FAILED(hr))
			{
				return false;
			}

			hr = (this->smtc_display_updater)->put_Thumbnail(thumbnail_pic_stream_reference.Get());
			if (FAILED(hr))
			{
				return false;
			}

			cover_loaded = true;
		}
	}

	if (!cover_loaded)
	{
		hr = (this->smtc_display_updater)->put_Thumbnail(nullptr);
		if (FAILED(hr))
		{
			return false;
		}
	}

	hr = (this->smtc_display_updater)->Update();
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool WindowsSystemMediaTransportControlsWrapper::clear_thumbnail()
{
	if (!initialized || this->smtc_display_updater == nullptr)
	{
		return false;
	}

	HRESULT result = (this->smtc_display_updater)->put_Thumbnail(nullptr);
	if (FAILED(result))
	{
		return false;
	}

	result = (this->smtc_display_updater)->Update();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool WindowsSystemMediaTransportControlsWrapper::clear_metadata()
{
	debug_log(L"clear_metadata: initialized=%d", initialized ? 1 : 0);
	if (!initialized || this->smtc_display_updater == nullptr)
	{
		return false;
	}

	clear_thumbnail();
	HRESULT hr = (this->smtc_display_updater)->ClearAll();
	if (FAILED(hr))
	{
		return false;
	}

	if (this->smtc == nullptr)
	{
		return false;
	}

	hr = (this->smtc)->put_IsEnabled(false);
	if (FAILED(hr))
	{
		return false;
	}
	this->smtc_enabled = false;
	return true;
}

HRESULT WindowsSystemMediaTransportControlsWrapper::button_pressed_callback(
	ISystemMediaTransportControls* sender,
	ISystemMediaTransportControlsButtonPressedEventArgs* args)
{
	PlaybackControls& playback_controls = WinampPlaybackWrapper::get_instance();
	SystemMediaControls& smtc = WindowsSystemMediaTransportControlsWrapper::get_instance();

	SystemMediaTransportControlsButton button;
	HRESULT hr = args->get_Button(&button);
	if (FAILED(hr))
	{
		debug_log(L"SMTC ButtonPressed: get_Button failed hr=0x%08X", (unsigned)hr);
		return hr;
	}

	debug_log(
		L"SMTC ButtonPressed: button=%d state_before=%d",
		(int)button,
		(int)WinampIpcWrapper::wa_ipc_is_playing());

	if (!media_key_action_allowed(media_key_action::from_smtc_button((int)button)))
	{
		// The same physical key press was already handled moments ago (e.g.
		// via WM_APPCOMMAND in the window procedure). Swallow it.
		debug_log(L"SMTC ButtonPressed: button=%d deduplicated", (int)button);
		return S_OK;
	}

	switch (button) {
	case SystemMediaTransportControlsButton_Play:
		playback_controls.play();
		smtc.set_playback_status(PlaybackState::playing);
		break;
	case SystemMediaTransportControlsButton_Pause:
		playback_controls.pause();
		smtc.set_playback_status(PlaybackState::paused);
		break;
	case SystemMediaTransportControlsButton_Next:
		playback_controls.next_track();
		break;
	case SystemMediaTransportControlsButton_Previous:
		playback_controls.previous_track();
		break;
	case SystemMediaTransportControlsButton_Stop:
		playback_controls.stop();
		smtc.set_playback_status(PlaybackState::stopped);
		break;
	case SystemMediaTransportControlsButton_Record:
	case SystemMediaTransportControlsButton_FastForward:
	case SystemMediaTransportControlsButton_Rewind:
	case SystemMediaTransportControlsButton_ChannelUp:
	case SystemMediaTransportControlsButton_ChannelDown:
		break;
	}

	debug_log(
		L"SMTC ButtonPressed: done button=%d state_after=%d",
		(int)button,
		(int)WinampIpcWrapper::wa_ipc_is_playing());

	return S_OK;
}

WindowsSystemMediaTransportControlsWrapper::~WindowsSystemMediaTransportControlsWrapper()
{
	(this->smtc)->remove_ButtonPressed((this->event_registration_token));
	this->WindowsSystemMediaTransportControlsWrapper::clear_metadata();
	this->window = nullptr;
}
