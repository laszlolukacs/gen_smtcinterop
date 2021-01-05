/* See LICENSE for copyright details. */

#include "windows_smtc_wrapper.h"


#include <cassert>
#include <systemmediatransportcontrolsinterop.h>
#include <windows.media.control.h>
#include <wrl/client.h>
#include <wrl/event.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Media;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

bool WindowsSystemMediaTransportControlsWrapper::create_hidden_smtc_window()
{
#ifdef _DEBUG
	std::wstring debug_message = L"Creating dummy window for SMTC...\n";
	OutputDebugString(debug_message.c_str());
#endif

	// the default hWnd what the plug-in was received from Winamp was insufficient for SMTC usage
	// so an appropriate invisible window is being created
	WNDCLASS smtc_wnd_class{};
	smtc_wnd_class.lpszClassName = smtc_wnd_class_name.c_str();
	smtc_wnd_class.hInstance = nullptr;
	smtc_wnd_class.lpfnWndProc = DefWindowProc;
	GetLastError();

	RegisterClass(&smtc_wnd_class);
	assert(!GetLastError());

	this->window = CreateWindowExW(
		0,
		smtc_wnd_class_name.c_str(),
		smtc_wnd_class_name.c_str(),
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		nullptr,
		nullptr,
		nullptr,
		nullptr);
	assert(this->window != nullptr);
	assert(!GetLastError());

	return true;
}

bool WindowsSystemMediaTransportControlsWrapper::destroy_hidden_smtc_window()
{
	if (this->window != nullptr)
	{
		if (!DestroyWindow(this->window)) {
#ifdef _DEBUG
			std::wstring debug_message = L"Failed to destroy the hidden SMTC window\n";
			OutputDebugString(debug_message.c_str());
#endif
		}

		if (!UnregisterClass(smtc_wnd_class_name.c_str(), nullptr)) {
#ifdef _DEBUG
			std::wstring debug_message = L"Failed to unregister the window class for hidden SMTC window\n";
			OutputDebugString(debug_message.c_str());
#endif
		}
	}

	return true;
}

bool WindowsSystemMediaTransportControlsWrapper::initialize()
{
	ComPtr<ISystemMediaTransportControlsInterop> smtc_interop;
	HRESULT hr = GetActivationFactory(
		HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(),
		smtc_interop.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	if (this->window == nullptr)
	{
		this->create_hidden_smtc_window();
	}

#ifdef _DEBUG
	assert(this->window != nullptr);
#endif

	hr = smtc_interop->GetForWindow((this->window), IID_PPV_ARGS((this->smtc).GetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	auto handler = Microsoft::WRL::Callback<	ITypedEventHandler<
		SystemMediaTransportControls*,
		SystemMediaTransportControlsButtonPressedEventArgs*>>(
			&WindowsSystemMediaTransportControlsWrapper::button_pressed_callback);

	hr = (this->smtc)->add_ButtonPressed(handler.Get(), &(this->event_registration_token));
	if (FAILED(hr))
		return false;

	hr = (this->smtc)->put_IsEnabled(true);
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc)->put_IsPlayEnabled(true);
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc)->put_IsPauseEnabled(true);
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc)->put_IsNextEnabled(true);
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc)->put_IsPreviousEnabled(true);
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc)->put_IsStopEnabled(true);
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc)->get_DisplayUpdater((this->smtc_display_updater).GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool WindowsSystemMediaTransportControlsWrapper::set_artist_and_track(std::wstring artist_name, std::wstring track_name)
{
	HRESULT hr = (this->smtc_display_updater)->put_Type(MediaPlaybackType_Music);
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
	return FAILED(hr);
}

bool WindowsSystemMediaTransportControlsWrapper::clear_metadata()
{
	HRESULT hr = (this->smtc_display_updater)->ClearAll();
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc)->put_IsEnabled(false);
	return FAILED(hr);
}

HRESULT WindowsSystemMediaTransportControlsWrapper::button_pressed_callback(
	ISystemMediaTransportControls* sender,
	ISystemMediaTransportControlsButtonPressedEventArgs* args)
{
	SystemMediaTransportControlsButton button;
	HRESULT hr = args->get_Button(&button);
	if (FAILED(hr))
		return hr;
	std::wstring debug_message = L"";

	switch (button) {
	case SystemMediaTransportControlsButton_Play:
		debug_message = L"Play Button was hit\n";
		OutputDebugString(debug_message.c_str());
		//impl->OnPlay();
		break;
	case SystemMediaTransportControlsButton_Pause:
		debug_message = L"Pause Button was hit\n";
		OutputDebugString(debug_message.c_str());
		//impl->OnPause();
		break;
	case SystemMediaTransportControlsButton_Next:
		debug_message = L"Next Button was hit\n";
		OutputDebugString(debug_message.c_str());
		//impl->OnNext();
		break;
	case SystemMediaTransportControlsButton_Previous:
		debug_message = L"Prev Button was hit\n";
		OutputDebugString(debug_message.c_str());
		//impl->OnPrevious();
		break;
	case SystemMediaTransportControlsButton_Stop:
		debug_message = L"Stop Button was hit\n";
		OutputDebugString(debug_message.c_str());
		/*impl->OnStop();*/
		break;
	case SystemMediaTransportControlsButton_Record:
	case SystemMediaTransportControlsButton_FastForward:
	case SystemMediaTransportControlsButton_Rewind:
	case SystemMediaTransportControlsButton_ChannelUp:
	case SystemMediaTransportControlsButton_ChannelDown:
		break;
	}

	return S_OK;
}

WindowsSystemMediaTransportControlsWrapper::~WindowsSystemMediaTransportControlsWrapper()
{
	(this->smtc)->remove_ButtonPressed((this->event_registration_token));
	this->clear_metadata();
	this->destroy_hidden_smtc_window();
}
