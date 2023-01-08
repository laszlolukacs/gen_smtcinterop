# gen_smtc

Windows [System Media Transport Controls](https://learn.microsoft.com/en-us/windows/uwp/audio-video-camera/integrate-with-systemmediatransportcontrols) Integration Plug-in for Winamp

## Dependencies
* [Visual C++ 2022 (v143)](https://visualstudio.microsoft.com/downloads/)
* [vcpkg](https://vcpkg.io/en/index.html)
* [Windows 10 SDK latest](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/)
* [Winamp 5.55 SDK](http://wiki.winamp.com/#Plug-in_Developer)
* [TagLib](https://taglib.org/) ([statically linked](https://levelup.gitconnected.com/how-to-statically-link-c-libraries-with-vcpkg-visual-studio-2019-435c2d4ace03))

## Summary of set up
* `git clone git@gitlab.com:laszlolukacs/gen_smtc.git`
* `[Environment]::SetEnvironmentVariable("WA_SDK_HOME", "C:\Program Files (x86)\Winamp SDK", [System.EnvironmentVariableTarget]::User)`
* `[Environment]::SetEnvironmentVariable("WINAMP_HOME", "C:\Program Files (x86)\Winamp", [System.EnvironmentVariableTarget]::User)`
* `vcpkg install taglib:x86-windows-static`

## Set the Winamp Plugins directory writable
To speed up debugging the plugin's newly built DLL could be automatically copied from the build directory to the Winamp installation's `Plugins` directory.
For this the Winamp installation's `Plugins` directory needs to be writable by the current user.
Executing the script below in an **Administrative** PowerShell session sets the `Plugins` directory writable: 
```(PowerShell)
$waPluginsPath = "C:\Program Files (x86)\Winamp\Plugins\"
$waPluginsAcl = Get-Acl $waPluginsPath
$waPluginsAclEntry = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name, "Write", "Allow"
$waPluginsAccessRule = New-Object System.Security.AccessControl.FileSystemAccessRule($waPluginsAclEntry)
$waPluginsAcl.SetAccessRule($waPluginsAccessRule)
$waPluginsAcl | Set-Acl $waPluginsPath
```
