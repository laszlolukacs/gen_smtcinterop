/* See LICENSE for copyright details. */

#include "windows_smtc_wrapper.h"

#ifdef _DEBUG
#include <cassert>
#endif
#include <SystemMediaTransportControlsInterop.h>
#include <windows.media.control.h>
#include <wrl/client.h>
#include <wrl/event.h>

#include "playback_controls.h"
#include "winamp_playback_wrapper.h"

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Media;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

bool WindowsSystemMediaTransportControlsWrapper::initialize(HWND hwnd)
{
	if (hwnd == nullptr)
	{
		return false;
	}

	this->window = hwnd;
	ComPtr<ISystemMediaTransportControlsInterop> smtc_interop;
	HRESULT hr = GetActivationFactory(
		HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(),
		smtc_interop.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	hr = smtc_interop->GetForWindow((this->window), IID_PPV_ARGS((this->smtc).GetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	auto handler = Microsoft::WRL::Callback<ITypedEventHandler<
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

	hr = (this->smtc)->
		put_PlaybackStatus(ABI::Windows::Media::MediaPlaybackStatus_Stopped);
	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc)->get_DisplayUpdater((this->smtc_display_updater).GetAddressOf());
	return FAILED(hr);
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

bool WindowsSystemMediaTransportControlsWrapper::set_playback_status(PlaybackState playback_status)
{
	HRESULT hr;
	switch (playback_status) {
	case PlaybackState::stopped:
		hr = (this->smtc)->
			put_PlaybackStatus(ABI::Windows::Media::MediaPlaybackStatus_Stopped);
		break;
	case PlaybackState::paused:
		hr = (this->smtc)->
			put_PlaybackStatus(ABI::Windows::Media::MediaPlaybackStatus_Paused);
		break;
	case PlaybackState::playing:
		hr = (this->smtc)->
			put_PlaybackStatus(ABI::Windows::Media::MediaPlaybackStatus_Playing);
		break;
	default:
		hr = (this->smtc)->
			put_PlaybackStatus(ABI::Windows::Media::MediaPlaybackStatus_Stopped);
	}

	if (FAILED(hr))
	{
		return false;
	}

	hr = (this->smtc_display_updater)->Update();
	return FAILED(hr);
}

bool WindowsSystemMediaTransportControlsWrapper::set_thumbnail(std::vector<unsigned char> thumbnail_bytes)
{

	HRESULT hr = ActivateInstance(
		HStringReference(RuntimeClass_Windows_Storage_Streams_InMemoryRandomAccessStream).Get(),
		mImageStream.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IOutputStream> outputStream;
	hr = mImageStream.As(&outputStream);
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IDataWriterFactory> dataWriterFactory;
	hr = GetActivationFactory(
		HStringReference(RuntimeClass_Windows_Storage_Streams_DataWriter).Get(),
		dataWriterFactory.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	hr = dataWriterFactory->CreateDataWriter(outputStream.Get(),
		mImageDataWriter.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	hr = mImageDataWriter->WriteBytes(
		thumbnail_bytes.size(), thumbnail_bytes.data());
	if (FAILED(hr))
	{
		return false;
	}

	hr = mImageDataWriter->StoreAsync(&mStoreAsyncOperation);
	if (FAILED(hr))
	{
		return false;
	}

	auto store_async_callback =
		Microsoft::WRL::Callback<ABI::Windows::Foundation::IAsyncOperationCompletedHandler<unsigned int>>(
			[this](ABI::Windows::Foundation::IAsyncOperation<unsigned int>* async_op, ABI::Windows::Foundation::AsyncStatus status) mutable
			{
				// Check the async operation completed successfully.
				ABI::Windows::Foundation::IAsyncInfo* async_info;
				HRESULT hr = async_op->QueryInterface(IID_IAsyncInfo, reinterpret_cast<void**>(&async_info));
				if (FAILED(hr))
				{
					return hr;
				}
				async_info->get_ErrorCode(&hr);
				if (SUCCEEDED(hr) && status == ABI::Windows::Foundation::AsyncStatus::Completed)
				{
					Microsoft::WRL::ComPtr<IRandomAccessStreamReferenceStatics>
						reference_statics;

					HRESULT	result = GetActivationFactory(
						HStringReference(RuntimeClass_Windows_Storage_Streams_RandomAccessStreamReference).Get(),
						reference_statics.GetAddressOf());
					if (FAILED(result))
					{
						return hr;
					}

					result = reference_statics->CreateFromStream(mImageStream.Get(),
						&mImageStreamReference);
					if (FAILED(result))
					{
						return hr;
					}

					result = (this->smtc_display_updater)->put_Thumbnail(mImageStreamReference.Get());
					if (FAILED(result))
					{
						return hr;
					}

					result = (this->smtc_display_updater)->Update();
					if (FAILED(result))
					{
						return hr;
					}
				}
				return hr;
			});

	hr = mStoreAsyncOperation->put_Completed(store_async_callback.Get());

	return false;
}

bool WindowsSystemMediaTransportControlsWrapper::clear_thumbnail()
{
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
}

bool WindowsSystemMediaTransportControlsWrapper::clear_metadata()
{
	clear_thumbnail();
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
	PlaybackControls& playback_controls = WinampPlaybackWrapper::get_instance();
	SystemMediaControls& smtc = WindowsSystemMediaTransportControlsWrapper::get_instance();

	SystemMediaTransportControlsButton button;
	HRESULT hr = args->get_Button(&button);
	if (FAILED(hr))
	{
		return hr;
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

	return S_OK;
}

WindowsSystemMediaTransportControlsWrapper::~WindowsSystemMediaTransportControlsWrapper()
{
	(this->smtc)->remove_ButtonPressed((this->event_registration_token));
	this->WindowsSystemMediaTransportControlsWrapper::clear_metadata();
	this->window = nullptr;
}
