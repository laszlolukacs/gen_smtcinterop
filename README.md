# gen_smtcinterop

Windows [System Media Transport Controls](https://learn.microsoft.com/en-us/windows/uwp/audio-video-camera/integrate-with-systemmediatransportcontrols) Interoperability Plug-in for Winamp

## Dependencies
* [Visual C++ 2019 (v142)](https://visualstudio.microsoft.com/downloads/)
* [vcpkg](https://vcpkg.io/en/getting-started.html)
* [Windows 10 SDK latest](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/)
* [Winamp 5.55 SDK](http://wiki.winamp.com/#Plug-in_Developer)
* [TagLib](https://taglib.org/) ([statically linked](https://levelup.gitconnected.com/how-to-statically-link-c-libraries-with-vcpkg-visual-studio-2019-435c2d4ace03))

## System requirements
* [Windows 10](https://www.microsoft.com/en-us/windows) or newer
* [Winamp 5.9.1](https://download.nullsoft.com/winamp/misc/Winamp591_10029_final_full_en-us.exe)
* [Visual C++ 2019 runtime](https://aka.ms/vs/17/release/vc_redist.x86.exe) (should be already installed along with Winamp)

## Summary of set up
* `git clone git@gitlab.com:laszlolukacs/gen_smtcinterop.git`
* `cd ./gen_smtcinterop`
* `vcpkg install taglib:x86-windows-static`
* Open `./gen_smtcinterop.sln` in Visual Studio

### Environment variables
* `WA_SDK_HOME` the root directory of the Winamp SDK, required to build the VC++ project
  *  `[Environment]::SetEnvironmentVariable("WA_SDK_HOME", "C:\Program Files (x86)\Winamp SDK", [System.EnvironmentVariableTarget]::User)`
* `WINAMP_HOME` the root directory of the Winamp installation itself
    * `[Environment]::SetEnvironmentVariable("WINAMP_HOME", "C:\Program Files (x86)\Winamp", [System.EnvironmentVariableTarget]::User)`

### Grant write permission on the Winamp Plugins directory
To speed up debugging the plugin's binary could be automatically copied from the build output directory to the Winamp installation's `Plugins` directory.
However for this to work the current user must have the write permission on the Winamp installation's `Plugins` directory.
Executing the script below in an **Administrative** PowerShell session grants exactly that: 
```(PowerShell)
$waPluginsPath = "C:\Program Files (x86)\Winamp\Plugins\"
$waPluginsAcl = Get-Acl $waPluginsPath
$waPluginsAclEntry = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name, "Write", "Allow"
$waPluginsAccessRule = New-Object System.Security.AccessControl.FileSystemAccessRule($waPluginsAclEntry)
$waPluginsAcl.SetAccessRule($waPluginsAccessRule)
$waPluginsAcl | Set-Acl $waPluginsPath
```
