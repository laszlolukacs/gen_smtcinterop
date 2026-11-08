/* See LICENSE for copyright details. */

#ifndef WINDOWS_RESOURCES_WRAPPER_H
#define WINDOWS_RESOURCES_WRAPPER_H

#include <windows.h>
#include <CommCtrl.h>
#include <vector>

#include "resource.h"

#pragma comment(lib, "ComCtl32.lib")

struct WindowsResourcesWrapper
{
	static void show_about_taskdialog(HINSTANCE hInstance, int& config_mode)
	{
		const TASKDIALOG_BUTTON radio_buttons[] =
		{
			{ CONFIG_MODE_ENABLED, MAKEINTRESOURCE(IDS_CONFIG_MODE_ENABLED) },
			{ CONFIG_MODE_ENABLED_OVERLAY, MAKEINTRESOURCE(IDS_CONFIG_MODE_ENABLED_OVERLAY) },
			{ CONFIG_MODE_DISABLED, MAKEINTRESOURCE(IDS_CONFIG_MODE_DISABLED) },
		};

		int nButtonPressed = 0;
		int nRadioButton = 0;
		TASKDIALOGCONFIG TaskDialogConfig = { 0 };
		TaskDialogConfig.cbSize = sizeof(TASKDIALOGCONFIG);
		TaskDialogConfig.hwndParent = NULL;
		TaskDialogConfig.hInstance = hInstance;
		TaskDialogConfig.dwFlags = TDF_SIZE_TO_CONTENT;
		TaskDialogConfig.dwCommonButtons = TDCBF_OK_BUTTON;
		TaskDialogConfig.pszWindowTitle = MAKEINTRESOURCE(IDS_ABOUT_WINDOW_TITLE);
		TaskDialogConfig.pszMainInstruction = MAKEINTRESOURCE(IDS_ABOUT_HEADER);
		TaskDialogConfig.pszContent = MAKEINTRESOURCE(IDS_ABOUT_CONTENT);
		TaskDialogConfig.pRadioButtons = radio_buttons;
		TaskDialogConfig.cRadioButtons = ARRAYSIZE(radio_buttons);
		TaskDialogConfig.nDefaultRadioButton = CONFIG_MODE_ENABLED + config_mode;

		const HRESULT hr = TaskDialogIndirect(&TaskDialogConfig, &nButtonPressed, &nRadioButton, NULL);
		if (SUCCEEDED(hr) &&
			nRadioButton >= CONFIG_MODE_ENABLED &&
			nRadioButton <= CONFIG_MODE_DISABLED)
		{
			config_mode = nRadioButton - CONFIG_MODE_ENABLED;
		}
	}

	static std::vector<unsigned char> get_default_album_art(LPCWSTR lpModuleName)
	{
		HMODULE module = nullptr;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			lpModuleName,
			&module);
		if (module == nullptr)
		{
			return std::vector<unsigned char>();
		}

		HRSRC bitmap_resource = FindResource(module, MAKEINTRESOURCE(IDB_NO_COVER_PNG), L"PNG");
		if (bitmap_resource == nullptr)
		{
			return std::vector<unsigned char>();
		}

		unsigned int resource_size = SizeofResource(module, bitmap_resource);
		HGLOBAL bitmap_resource_data = LoadResource(module, bitmap_resource);
		if (bitmap_resource_data == nullptr)
		{
			return std::vector<unsigned char>();
		}

		void* resource_pixels = LockResource(bitmap_resource_data);
		return std::vector<unsigned char>((unsigned char*)resource_pixels, (unsigned char*)resource_pixels + resource_size);
	}
};

#endif // WINDOWS_RESOURCES_WRAPPER_H