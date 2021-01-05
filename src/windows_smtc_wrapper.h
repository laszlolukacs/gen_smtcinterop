#ifndef WINDOWS_SMTC_WRAPPER_H
#define WINDOWS_SMTC_WRAPPER_H

#include <windows.foundation.h>
#include <windows.media.control.h>
#include <wrl/client.h>

#include "../system_media_controls.h"

using Microsoft::WRL::ComPtr;

class WindowsSystemMediaTransportControlsWrapper : public SystemMediaControls
{
private:
	const std::wstring smtc_wnd_class_name = L"Winamp-gen_smtc-1zz";

	HWND window = nullptr;

	ComPtr<ABI::Windows::Media::ISystemMediaTransportControls>
		smtc;

	ComPtr<ABI::Windows::Media::ISystemMediaTransportControlsDisplayUpdater>
		smtc_display_updater;

	ComPtr<ABI::Windows::Media::IMusicDisplayProperties>
		smtc_music_display_properties;

	EventRegistrationToken event_registration_token;

	static HRESULT button_pressed_callback(
		ABI::Windows::Media::ISystemMediaTransportControls* sender,
		ABI::Windows::Media::ISystemMediaTransportControlsButtonPressedEventArgs* args);

	bool create_hidden_smtc_window();
	bool destroy_hidden_smtc_window();

public:
	bool initialize() override;
	bool set_artist_and_track(std::wstring artist_name, std::wstring track_name) override;
	bool clear_metadata() override;

	~WindowsSystemMediaTransportControlsWrapper() override;
};

#endif // WINDOWS_SMTC_WRAPPER_H
