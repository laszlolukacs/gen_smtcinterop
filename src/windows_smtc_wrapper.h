/* See LICENSE for copyright details. */

#ifndef WINDOWS_SMTC_WRAPPER_H
#define WINDOWS_SMTC_WRAPPER_H

#include <windows.foundation.h>
#include <windows.media.control.h>
#include <wrl/client.h>

#include "system_media_controls.h"

using Microsoft::WRL::ComPtr;

class WindowsSystemMediaTransportControlsWrapper : public SystemMediaControls
{
private:
	WindowsSystemMediaTransportControlsWrapper() { }

	bool initialized = false;

	HWND window = nullptr;

	ComPtr<ABI::Windows::Media::ISystemMediaTransportControls>
		smtc;

	ComPtr<ABI::Windows::Media::ISystemMediaTransportControlsDisplayUpdater>
		smtc_display_updater;

	ComPtr<ABI::Windows::Media::IMusicDisplayProperties>
		smtc_music_display_properties;

	EventRegistrationToken event_registration_token = EventRegistrationToken{ 0 };

	static HRESULT button_pressed_callback(
		ABI::Windows::Media::ISystemMediaTransportControls* sender,
		ABI::Windows::Media::ISystemMediaTransportControlsButtonPressedEventArgs* args);

	bool set_thumbnail_sync(std::vector<unsigned char> thumbnail_bytes);
	bool set_thumbnail_async(std::vector<unsigned char> thumbnail_bytes);

public:
	static WindowsSystemMediaTransportControlsWrapper& get_instance()
	{
		static WindowsSystemMediaTransportControlsWrapper instance;
		return instance;
	}

	WindowsSystemMediaTransportControlsWrapper(
		WindowsSystemMediaTransportControlsWrapper const&) = delete;
	void operator=(
		WindowsSystemMediaTransportControlsWrapper const&) = delete;

	bool initialize(HWND = nullptr) override;
	bool set_artist_and_track(std::wstring artist_name, std::wstring track_name) override;
	bool set_playback_status(PlaybackState playback_status) override;
	bool set_thumbnail(std::vector<unsigned char> thumbnail_bytes) override;
	bool clear_thumbnail() override;
	bool clear_metadata() override;

	~WindowsSystemMediaTransportControlsWrapper() override;
};

#endif // WINDOWS_SMTC_WRAPPER_H
