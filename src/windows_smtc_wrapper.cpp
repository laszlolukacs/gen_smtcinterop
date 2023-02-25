/* See LICENSE for copyright details. */

#include "windows_smtc_wrapper.h"

#ifdef _DEBUG
#include <cassert>
#endif
#include <SystemMediaTransportControlsInterop.h>
#include <wrl/event.h>
#include <shcore.h>
#include <Shlwapi.h>

#include "playback_controls.h"
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

	return true;
}

bool WindowsSystemMediaTransportControlsWrapper::set_artist_and_track(std::wstring artist_name, std::wstring track_name)
{
	if (!initialized || this->smtc_display_updater == nullptr)
	{
		return false;
	}

	HRESULT hr = (this->smtc)->put_IsEnabled(true);
	if (FAILED(hr))
	{
		return false;
	}

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
	return FAILED(hr);
}

bool WindowsSystemMediaTransportControlsWrapper::set_playback_status(PlaybackState playback_status)
{
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
	return FAILED(hr);
}

bool WindowsSystemMediaTransportControlsWrapper::set_thumbnail(std::vector<unsigned char> thumbnail_bytes)
{
	if (!initialized || this->smtc_display_updater == nullptr)
	{
		return false;
	}

	if (thumbnail_bytes.empty())
	{
		return this->clear_thumbnail();
	}
	else
	{
		return this->set_thumbnail_sync(thumbnail_bytes);
	}
}

bool WindowsSystemMediaTransportControlsWrapper::set_thumbnail_sync(std::vector<unsigned char> thumbnail_bytes)
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

bool WindowsSystemMediaTransportControlsWrapper::set_thumbnail_async(std::vector<unsigned char> thumbnail_bytes)
{
	if (!initialized || this->smtc_display_updater == nullptr)
	{
		return false;
	}

	ComPtr<IRandomAccessStream> thumbnail_pic_stream;
	HRESULT hr = ActivateInstance(
		HStringReference(RuntimeClass_Windows_Storage_Streams_InMemoryRandomAccessStream).Get(),
		thumbnail_pic_stream.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IOutputStream> output_stream;
	hr = thumbnail_pic_stream.As(&output_stream);
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IDataWriterFactory> data_writer_factory;
	hr = GetActivationFactory(
		HStringReference(RuntimeClass_Windows_Storage_Streams_DataWriter).Get(),
		data_writer_factory.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IDataWriter>	thumbnail_pic_data_writer;
	hr = data_writer_factory->CreateDataWriter(output_stream.Get(),
		thumbnail_pic_data_writer.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	hr = thumbnail_pic_data_writer->WriteBytes(
		thumbnail_bytes.size(), thumbnail_bytes.data());
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IAsyncOperation<unsigned int>> store_async_operation;
	hr = thumbnail_pic_data_writer->StoreAsync(&store_async_operation);
	if (FAILED(hr))
	{
		return false;
	}

	auto store_async_callback =
		Callback<IAsyncOperationCompletedHandler<unsigned int>>(
			[&](IAsyncOperation<unsigned int>* async_op,
				AsyncStatus async_status) mutable
			{
				// Check the async operation completed successfully.
				IAsyncInfo* async_info;
				HRESULT hr = async_op->QueryInterface(IID_IAsyncInfo, reinterpret_cast<void**>(&async_info));
				if (FAILED(hr))
				{
					return hr;
				}

				async_info->get_ErrorCode(&hr);
				if (SUCCEEDED(hr) && async_status == AsyncStatus::Completed)
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

					ComPtr<IRandomAccessStreamReference> thumbnail_pic_stream_reference;
					result = reference_statics->CreateFromStream(thumbnail_pic_stream.Get(),
						&thumbnail_pic_stream_reference);
					if (FAILED(result))
					{
						return hr;
					}

					result = (this->smtc_display_updater)->put_Thumbnail(thumbnail_pic_stream_reference.Get());
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

	hr = store_async_operation->put_Completed(store_async_callback.Get());
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
