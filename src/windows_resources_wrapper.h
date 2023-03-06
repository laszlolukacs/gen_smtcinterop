/* See LICENSE for copyright details. */

#ifndef WINDOWS_RESOURCES_WRAPPER_H
#define WINDOWS_RESOURCES_WRAPPER_H

#include <windows.h>
#include <CommCtrl.h>
#include <VersionHelpers.h>

#include "resource.h"

#pragma comment(lib, "ComCtl32.lib")

struct WindowsResourcesWrapper
{
	static void show_about_taskdialog(HINSTANCE hInstance)
	{
		int nButtonPressed = 0;
		TASKDIALOGCONFIG TaskDialogConfig = { 0 };
		TaskDialogConfig.cbSize = sizeof(TASKDIALOGCONFIG);
		TaskDialogConfig.hwndParent = NULL;
		TaskDialogConfig.hInstance = hInstance;
		TaskDialogConfig.dwFlags = TDF_SIZE_TO_CONTENT;
		TaskDialogConfig.dwCommonButtons = TDCBF_OK_BUTTON;
		TaskDialogConfig.pszWindowTitle = MAKEINTRESOURCE(IDS_ABOUT_WINDOW_TITLE);
		TaskDialogConfig.pszMainInstruction = MAKEINTRESOURCE(IDS_ABOUT_HEADER);
		TaskDialogConfig.pszContent = MAKEINTRESOURCE(IDS_ABOUT_CONTENT);
		if (!IsWindows10OrGreater())
		{
			TaskDialogConfig.pszFooter = MAKEINTRESOURCE(IDS_WIN_VER_TOO_LOW);
			TaskDialogConfig.pszFooterIcon = TD_ERROR_ICON;
		}

		TaskDialogIndirect(&TaskDialogConfig, &nButtonPressed, NULL, NULL);
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