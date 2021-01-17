/* See LICENSE for copyright details. */

#include "winamp_playback_wrapper.h"

#include <wa_ipc.h>

#define WINAMP_PREVIOUS_TRACK_PARAM 40044
#define WINAMP_START_PLAYBACK_PARAM 40045
#define WINAMP_STOP_PLAYBACK_PARAM 40047
#define WINAMP_NEXT_TRACK_PARAM 40048

void WinampPlaybackWrapper::set_window(HWND window)
{
	this->window = window;
}

void WinampPlaybackWrapper::play()
{
	if (this->window == nullptr)
	{
		return;
	}

	const int is_playing = SendMessage(
		this->window,
		WM_USER,
		0,
		IPC_ISPLAYING) == 1;

	SendMessage(
		this->window,
		WM_COMMAND,
		WINAMP_START_PLAYBACK_PARAM + is_playing,
		0);
}

void WinampPlaybackWrapper::pause()
{
	if (this->window == nullptr)
	{
		return;
	}

	this->play();
}

void WinampPlaybackWrapper::next_track()
{
	if (this->window == nullptr)
	{
		return;
	}

	SendMessage(
		this->window,
		WM_COMMAND,
		WINAMP_NEXT_TRACK_PARAM,
		0);
}

void WinampPlaybackWrapper::previous_track()
{
	if (this->window == nullptr)
	{
		return;
	}

	SendMessage(
		this->window,
		WM_COMMAND,
		WINAMP_PREVIOUS_TRACK_PARAM,
		0);
}

void WinampPlaybackWrapper::stop()
{
	if (this->window == nullptr)
	{
		return;
	}

	SendMessage(
		this->window,
		WM_COMMAND,
		WINAMP_STOP_PLAYBACK_PARAM,
		0);
}
