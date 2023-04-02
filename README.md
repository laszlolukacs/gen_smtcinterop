# gen_smtcinterop

Windows [System Media Transport Controls](https://learn.microsoft.com/en-us/windows/uwp/audio-video-camera/integrate-with-systemmediatransportcontrols) Interoperability Plug-in for Winamp

![Winamp and System Media Transport Controls showcase](docs/screenshots/showcase001.gif?raw=true)

Propagates the metadata of all contents that Winamp supports to SMTC and lets you focus and control Winamp from the Windows Media Controls. Provides the functionality via a single binary (DLL).

## System requirements
* [Windows 10](https://www.microsoft.com/en-us/windows) or newer
* [Winamp 5.9.1](https://download.nullsoft.com/winamp/misc/Winamp591_10029_final_full_en-us.exe)
* [Visual C++ 2019 runtime](https://aka.ms/vs/17/release/vc_redist.x86.exe) (should be already installed along with Winamp 5.9.1)

## Installation
1. Download the plugin from Releases
2. Copy the `gen_smtcinterop.dll` to your Winamp installation's `Plugins` directory

## Dependencies
* [Visual C++ 2019 (v142)](https://visualstudio.microsoft.com/downloads/)
* [vcpkg](https://vcpkg.io/en/getting-started.html)
  * must be integrated with MSBuild and Visual Studio using `vcpkg integrate install` to be able to reference TagLib
  * recommended to add `vcpkg` to `PATH`
* [Windows 10 SDK latest](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/)
* [Winamp 5.55 SDK](http://wiki.winamp.com/#Plug-in_Developer)
* [TagLib](https://taglib.org/) ([statically linked](https://levelup.gitconnected.com/how-to-statically-link-c-libraries-with-vcpkg-visual-studio-2019-435c2d4ace03))

## Summary of set up
* `git clone git@github.com:laszlolukacs/gen_smtcinterop.git`
* `cd ./gen_smtcinterop`
* `vcpkg install taglib:x86-windows-static`
* Open `./gen_smtcinterop.sln` in Visual Studio

### Environment variables
* `WA_SDK_HOME` the root directory of the Winamp SDK, required to build the VC++ project
  *  `[Environment]::SetEnvironmentVariable("WA_SDK_HOME", "C:\Program Files (x86)\Winamp SDK", [System.EnvironmentVariableTarget]::User)`
* `WINAMP_HOME` the root directory of the Winamp installation itself
    * `[Environment]::SetEnvironmentVariable("WINAMP_HOME", "C:\Program Files (x86)\Winamp", [System.EnvironmentVariableTarget]::User)`

### Grant write permission on the Winamp Plugins directory
The plugin's binary is automatically copied from the build output directory to the Winamp installation's `Plugins` directory for debug builds. 
However for this to work the current user must have the write permission on the Winamp installation's `Plugins` directory. This could be set from the Windows Explorer. 
Alternatively executing the script below in an **Administrative** PowerShell session grants that: 
```(PowerShell)
$waPluginsPath = "C:\Program Files (x86)\Winamp\Plugins\"
$waPluginsAcl = Get-Acl $waPluginsPath
$waPluginsAclEntry = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name, "Write", "Allow"
$waPluginsAccessRule = New-Object System.Security.AccessControl.FileSystemAccessRule($waPluginsAclEntry)
$waPluginsAcl.SetAccessRule($waPluginsAccessRule)
$waPluginsAcl | Set-Acl $waPluginsPath
Get-Acl -Path $waPluginsPath
```

## Greetings
* [NanMetal/gen_smtc](https://github.com/NanMetal/gen_smtc) - A longer existing Winamp plugin delivering the same SMTC integration, the major difference is instead of the VC++ Runtime it depends on the .NET Framework
