/* See LICENSE for copyright details. */

#ifndef GEN_SMTC_H
#define GEN_SMTC_H
#include <windows.h>

// plugin version (don't touch this)
#define GPPHDR_VER 0x10

// plugin name/title (change this to something you like)
#define PLUGIN_NAME "System Media Transport Controls Integration Plug-in"

/**
 * Handles the `init` event of a Winamp GPP.
 */
int init(void);

/**
 * Handles the `config` event of a Winamp GPP.
 */
void config(void);

/**
 * Handles the `quit` event of a Winamp GPP.
 */
void quit(void);

extern "C"
{
	// Winamp GPP (General Purpose Plugin) structure
	typedef struct
	{
		int version;            // Version of the plugin structure
		char* description;      // Name of the plugin
		int(*init)();           // Function which will be executed on the `init` event
		void(*config)();        // Function which will be executed on the `config` event
		void(*quit)();          // Function which will be executed on the `quit` event
		HWND hwndParent;        // handle of the Winamp main window (stored by Winamp when DLL is loaded)
		HINSTANCE hDllInstance; // handle of this plugin DLL instance (stored by Winamp when DLL is loaded)
	} winampGeneralPurposePlugin;
}

#endif // GEN_SMTC_H
