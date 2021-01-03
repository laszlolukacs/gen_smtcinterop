#ifndef WINDOWS_SMTC_WRAPPER_H
#define WINDOWS_SMTC_WRAPPER_H

#  include <functional>
#  include <windows.h>
#  include <windows.media.h>
#  include <winsdkver.h>
#  include <wrl.h>

using ISMTC = ABI::Windows::Media::ISystemMediaTransportControls;
using SMTCProperty = ABI::Windows::Media::SystemMediaTransportControlsProperty;
using ISMTCDisplayUpdater = ABI::Windows::Media::ISystemMediaTransportControlsDisplayUpdater;

using Microsoft::WRL::ComPtr;

extern ComPtr<ISMTC> mControls;
extern ComPtr<ISMTCDisplayUpdater> mDisplay;

bool Create(HWND hwnd);

#endif // WINDOWS_SMTC_WRAPPER_H
