/* See LICENSE for copyright details. */

#include "winamp_playback_wrapper.h"

#include <wa_ipc.h>

#define WINAMP_PREVIOUS_TRACK_PARAM 40044
#define WINAMP_START_PLAYBACK_PARAM 40045
#define WINAMP_PAUSE_PLAYBACK_PARAM 40046
#define WINAMP_STOP_PLAYBACK_PARAM 40047
#define WINAMP_NEXT_TRACK_PARAM 40048

void WinampPlaybackWrapper::set_window(HWND window)
{
	this->window = window;
}

bool WinampPlaybackWrapper::is_playing() const
{
	if (this->window == nullptr)
	{
		return false;
	}

	return SendMessage(
		this->window,
		WM_WA_IPC,
		0,
		IPC_ISPLAYING) == 1;
}

void WinampPlaybackWrapper::play()
{
	// Play must be idempotent: SMTC and Winamp can both act on the same media
	// key, and sending the Play command while already playing would restart
	// the current track.
	const bool playing = this->is_playing();
	if (this->window == nullptr || playing)
	{
		return;
	}

	SendMessage(
		this->window,
		WM_COMMAND,
		WINAMP_START_PLAYBACK_PARAM,
		0);
}

void WinampPlaybackWrapper::pause()
{
	// Pause must be absolute, not a toggle: a hardware play/pause key can
	// deliver both a Play and a Pause command, and toggling here would resume
	// playback again.
	const bool playing = this->is_playing();
	if (this->window == nullptr || !playing)
	{
		return;
	}

	SendMessage(
		this->window,
		WM_COMMAND,
		WINAMP_PAUSE_PLAYBACK_PARAM,
		0);
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
