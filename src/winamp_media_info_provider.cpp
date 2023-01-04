/* See LICENSE for copyright details. */

#include "winamp_media_info_provider.h"

#include <cassert>
#include "winamp_ipc_wrapper.h"

void WinampMediaInfoProvider::set_window(HWND window)
{
	this->window = window;
}

void WinampMediaInfoProvider::init_winamp_services()
{
	if (this->window == nullptr)
	{
		return;
	}

	api_service* wasabi_service_manager = reinterpret_cast<api_service*>(SendMessage(this->window, WM_WA_IPC, 0, IPC_GET_API_SERVICE));
	if (wasabi_service_manager == reinterpret_cast<api_service*>(1)
		|| wasabi_service_manager == nullptr)
	{
		wasabi_service_manager = nullptr;
	}
	else if (wasabi_service_manager != nullptr)
	{
		waServiceFactory* memory_manager_factory = wasabi_service_manager->service_getServiceByGuid(memMgrApiServiceGuid);
		if (memory_manager_factory != nullptr)
		{
			this->wasabi_memory_manager = reinterpret_cast<api_memmgr*>(memory_manager_factory->getInterface());
#ifdef _DEBUG
			assert(this->wasabi_memory_manager != nullptr);
#endif
		}

		waServiceFactory* album_art_factory = wasabi_service_manager->service_enumService(svc_albumArtProvider::getServiceType(), 0);
		if (album_art_factory != nullptr)
		{
			this->album_art_provider = reinterpret_cast<svc_albumArtProvider*>(album_art_factory->getInterface());
#ifdef _DEBUG
			assert(this->album_art_provider != nullptr);
#endif
		}
	}
}

MediaInfo WinampMediaInfoProvider::get_metadata_of_song(std::wstring filename)
{
	const wchar_t* current_filename = filename.c_str();
	if (current_filename != nullptr)
	{
		wchar_t artist_name[256];
		const bool received_artist = WinampIpcWrapper::wa_ipc_get_file_metadata(
			current_filename,
			WINAMP_METADATA_ARTIST,
			artist_name,
			256) && artist_name[0];

		wchar_t song_title[256];
		const bool received_title = WinampIpcWrapper::wa_ipc_get_file_metadata(
			current_filename,
			WINAMP_METADATA_TITLE,
			song_title,
			256) && song_title[0];

		wasabi_get_album_art(current_filename);

		if (received_title)
		{
			if (!received_artist)
			{
				return MediaInfo(L"", &song_title[0]);
			}

			return MediaInfo(&artist_name[0], &song_title[0]);
		}
	}

	return MediaInfo(L"", L"");
}

unsigned int* WinampMediaInfoProvider::wasabi_get_album_art(const wchar_t* filename) const
{
	if (this->album_art_provider == nullptr)
	{
		return nullptr;
	}

	void* album_art_pixels = nullptr;
	size_t album_art_size = 0;
	wchar_t* album_art_mime_type = nullptr;

#ifdef _DEBUG
	assert(album_art_provider != nullptr);
#endif
	bool isMine = album_art_provider->IsMine(filename);

	int type = album_art_provider->ProviderType();

	int result = album_art_provider->GetAlbumArtData(filename, L"cover", &album_art_pixels, &album_art_size, &album_art_mime_type);
	if (result != ALBUMARTPROVIDER_SUCCESS)
	{
#ifdef _DEBUG
		std::wstring debug_message = L"album_art_provider->GetAlbumArtData FAILED TO LOAD ALBUM ART\n";
		OutputDebugString(debug_message.c_str());
#endif
	}
	else
	{
#ifdef _DEBUG
		std::wstring debug_message = L"album_art_provider->GetAlbumArtData SUCCESSFULLY LOADED ALBUM ART\n";
		OutputDebugString(debug_message.c_str());
#endif
	}

	return nullptr;
}

void WinampMediaInfoProvider::deregister_winamp_services()
{
	if (this->window == nullptr)
	{
		return;
	}

	api_service* wasabi_service_manager = reinterpret_cast<api_service*>(SendMessage(this->window, WM_WA_IPC, 0, IPC_GET_API_SERVICE));
	if (wasabi_service_manager == reinterpret_cast<api_service*>(1)
		|| wasabi_service_manager == nullptr)
	{
		wasabi_service_manager = nullptr;
	}
	else if (wasabi_service_manager != nullptr)
	{
		waServiceFactory* memory_manager_factory = wasabi_service_manager->service_getServiceByGuid(memMgrApiServiceGuid);
		if (memory_manager_factory != nullptr
			&& this->wasabi_memory_manager != nullptr)
		{
			memory_manager_factory->releaseInterface(this->wasabi_memory_manager);
		}


		waServiceFactory* album_art_factory = wasabi_service_manager->service_enumService(svc_albumArtProvider::getServiceType(), 0);
		if (album_art_factory != nullptr)
		{
			wasabi_service_manager->service_deregister(album_art_factory);
		}
	}

	this->album_art_provider = nullptr;
	this->wasabi_memory_manager = nullptr;
	this->window = nullptr;
}
