/* See LICENSE for copyright details. */

#ifndef WINAMP_PLAYBACK_WRAPPER_H
#define WINAMP_PLAYBACK_WRAPPER_H

#include <windows.h>

#include "playback_controls.h"

class WinampPlaybackWrapper : public PlaybackControls
{
private:
	WinampPlaybackWrapper() { }

	HWND window = nullptr;

public:
	static WinampPlaybackWrapper& get_instance()
	{
		static WinampPlaybackWrapper instance;
		return instance;
	}

	WinampPlaybackWrapper(WinampPlaybackWrapper const&) = delete;
	void operator=(WinampPlaybackWrapper const&) = delete;

	void set_window(HWND window);
	void play() override;
	void pause() override;
	void next_track() override;
	void previous_track() override;
	void stop() override;
};

#endif
