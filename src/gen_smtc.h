/* See LICENSE for copyright details. */

#ifndef GEN_SMTC_H
#define GEN_SMTC_H
#include <windows.h>

// plugin version (don't touch this)
#define GPPHDR_VER 0x10

// plugin name/title (change this to something you like)
#define PLUGIN_NAME "System Media Transport Controls Plug-in"

extern "C"
{
	// these are callback functions/events which will be called by Winamp
	int  init(void);
	void config(void);
	void quit(void);

	// main structure with plugin information, version, name...
	typedef struct
	{
		int version;            // version of the plugin structure
		char* description;      // name of the plugin
		int(*init)();           // function which will be executed on init event
		void(*config)();        // function which will be executed on config event
		void(*quit)();          // function which will be executed on quit event
		HWND hwndParent;        // handle of the Winamp main window (stored by Winamp when DLL is loaded)
		HINSTANCE hDllInstance; // handle of this plugin DLL instance (stored by Winamp when DLL is loaded)
	} winampGeneralPurposePlugin;
}

#endif // GEN_SMTC_H
