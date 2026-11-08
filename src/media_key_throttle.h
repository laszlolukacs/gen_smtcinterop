/* See LICENSE for copyright details. */

#ifndef MEDIA_KEY_THROTTLE_H
#define MEDIA_KEY_THROTTLE_H

#include <windows.h>

#include <atomic>

// Canonical media key actions. A single physical key press can be delivered
// through two different paths (a WM_APPCOMMAND message to a Winamp window
// and the SMTC button pressed callback). Canonicalizing both paths to the
// same action allows deduplicating them.
namespace media_key_action
{
	enum type
	{
		none = 0,
		play_pause, // Play, Pause and Play/Pause intentionally share one slot
		stop,
		next_track,
		previous_track,
		fast_forward,
		rewind,
		count
	};

	inline type from_appcommand(WORD appcommand)
	{
		switch (appcommand)
		{
		case APPCOMMAND_MEDIA_PLAY:
		case APPCOMMAND_MEDIA_PAUSE:
		case APPCOMMAND_MEDIA_PLAY_PAUSE:
			return play_pause;
		case APPCOMMAND_MEDIA_STOP:
			return stop;
		case APPCOMMAND_MEDIA_NEXTTRACK:
			return next_track;
		case APPCOMMAND_MEDIA_PREVIOUSTRACK:
			return previous_track;
		case APPCOMMAND_MEDIA_FAST_FORWARD:
			return fast_forward;
		case APPCOMMAND_MEDIA_REWIND:
			return rewind;
		default:
			return none;
		}
	}

	// SystemMediaTransportControlsButton values:
	// Play=0, Pause=1, Stop=2, Record=3, FastForward=4, Rewind=5, Next=6, Previous=7
	inline type from_smtc_button(int button)
	{
		switch (button)
		{
		case 0: // Play
		case 1: // Pause
			return play_pause;
		case 2: // Stop
			return stop;
		case 4: // FastForward
			return fast_forward;
		case 5: // Rewind
			return rewind;
		case 6: // Next
			return next_track;
		case 7: // Previous
			return previous_track;
		default:
			return none;
		}
	}
}

// Returns true if the action should be performed. Returns false when the
// same action was already performed within the last few milliseconds, which
// means the same physical key press arrived through a second path.
inline bool media_key_action_allowed(media_key_action::type action)
{
	const DWORD dedupe_window_ms = 200;

	if (action <= media_key_action::none || action >= media_key_action::count)
	{
		return true;
	}

	// Function-local statics of an inline function are shared by all
	// translation units (C++17), so both delivery paths use the same state.
	static std::atomic<DWORD> last_action_tick[media_key_action::count] = {};
	static std::atomic<bool> last_action_valid[media_key_action::count] = {};

	const DWORD now = GetTickCount();

	// Atomically check-and-set so that two delivery paths running on
	// different threads (window procedure vs. SMTC event callback) cannot
	// both pass.
	const DWORD last_tick = last_action_tick[action].exchange(now);
	const bool was_valid = last_action_valid[action].exchange(true);

	return !was_valid || (now - last_tick) > dedupe_window_ms;
}

#endif // MEDIA_KEY_THROTTLE_H
