/* See LICENSE for copyright details. */

#include "windows_smtc_wrapper.h"

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Media;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

#  ifndef ISystemMediaTransportControlsInterop
EXTERN_C const IID IID_ISystemMediaTransportControlsInterop;
MIDL_INTERFACE("ddb0472d-c911-4a1f-86d9-dc3d71a95f5a")
ISystemMediaTransportControlsInterop : public IInspectable{
 public:
  virtual HRESULT STDMETHODCALLTYPE GetForWindow(
	  /* [in] */ __RPC__in HWND appWindow,
	  /* [in] */ __RPC__in REFIID riid,
	  /* [iid_is][retval][out] */  __RPC__deref_out_opt void** mediaTransportControl) = 0;
};
#  endif /* __ISystemMediaTransportControlsInterop_INTERFACE_DEFINED__ */

bool Create(HWND hwnd)
{
	ComPtr<ISystemMediaTransportControlsInterop> interop;
	HRESULT hr = GetActivationFactory(
		HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(),
		interop.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	hr = interop->GetForWindow(hwnd, IID_PPV_ARGS(mControls.GetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	hr = mControls->get_DisplayUpdater(mDisplay.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}
