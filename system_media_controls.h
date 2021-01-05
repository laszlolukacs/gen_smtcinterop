#ifndef SYSTEM_MEDIA_CONTROLS_H
#define SYSTEM_MEDIA_CONTROLS_H

#include <string>

struct SystemMediaControls {
	SystemMediaControls() = default;
	SystemMediaControls(const SystemMediaControls&) = default;
	SystemMediaControls(SystemMediaControls&&) = default;
	SystemMediaControls& operator=(const SystemMediaControls&) = default;
	SystemMediaControls& operator=(SystemMediaControls&&) = default;
	virtual ~SystemMediaControls() = default;

	virtual bool initialize() = 0;
	virtual bool set_artist_and_track(std::wstring artist_name, std::wstring track_name) = 0;
	virtual bool clear_metadata() = 0;
};

#endif
