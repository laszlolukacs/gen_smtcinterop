/* See LICENSE for copyright details. */

#ifndef SYSTEM_MEDIA_CONTROLS_H
#define SYSTEM_MEDIA_CONTROLS_H

#include <string>
#include <vector>
#include <windows.h>

#include "playback_state.h"

struct SystemMediaControls
{
	virtual bool initialize(HWND = nullptr) = 0;
	virtual bool set_artist_and_track(std::wstring artist_name, std::wstring track_name) = 0;
	virtual bool set_thumbnail(std::vector<unsigned char> thumbnail_bytes) = 0;
	virtual bool set_playback_status(PlaybackState playback_status) = 0;
	virtual bool clear_thumbnail() = 0;
	virtual bool clear_metadata() = 0;

	virtual ~SystemMediaControls() = default;
};

#endif
