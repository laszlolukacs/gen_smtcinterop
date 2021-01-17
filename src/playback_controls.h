/* See LICENSE for copyright details. */

#ifndef PLAYBACK_CONTROLS_H
#define PLAYBACK_CONTROLS_H

struct PlaybackControls
{
	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void next_track() = 0;
	virtual void previous_track() = 0;
	virtual void stop() = 0;

	virtual ~PlaybackControls() = default;
};

#endif // PLAYBACK_CONTROLS_H
