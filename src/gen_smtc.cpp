/* See LICENSE for copyright details. */

#include "gen_smtc.h"

// GPPHDR_VER is the version of the winampGeneralPurposePlugin (GPP) structure
winampGeneralPurposePlugin plugin = {
	GPPHDR_VER,  // version of the plugin, defined in the header
	PLUGIN_NAME, // name of the plugin, defined in the header
	init,        // function name which will be executed on init event
	config,      // function name which will be executed on config event
	quit,        // function name which will be executed on quit event
	0,           // handle of the Winamp main window (stored by Winamp when DLL is loaded)
	0            // handle of this plugin DLL instance (stored by Winamp when DLL is loaded)
};

// event functions follow

int init() {
	//A basic messagebox that tells you the 'init' event has been triggered.
	//If everything works you should see this message when you start Winamp once your plugin has been installed.
	//You can change this later to do whatever you want (including nothing)
	MessageBox(plugin.hwndParent, L"Init event triggered for gen_smtc. Plugin installed successfully!", L"", MB_OK);
	return 0;
}

void config() {
	//A basic messagebox that tells you the 'config' event has been triggered.
	//You can change this later to do whatever you want (including nothing)
	MessageBox(plugin.hwndParent, L"Config event triggered for gen_smtc.", L"", MB_OK);
}

void quit() {
	//A basic messagebox that tells you the 'quit' event has been triggered.
	//If everything works you should see this message when you quit Winamp once your plugin has been installed.
	//You can change this later to do whatever you want (including nothing)
	MessageBox(0, L"Quit event triggered for gen_smtc.", L"", MB_OK);
}


// This is an export function called by winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() {
	return &plugin;
}
