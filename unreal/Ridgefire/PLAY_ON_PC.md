# Play IRON SUN on Windows

1. Download or copy the **entire** packaged `Windows` folder, then unzip it before playing. Do not run the executable from inside a ZIP.
2. Open `Windows/TP_FirstPerson.exe`. Keep the adjacent `TP_FirstPerson/` content folder beside it.
3. If Windows reports a missing Visual C++ runtime, run the included `UEPrereqSetup_x64.exe` if it is present in `Windows/Engine/Extras/Redist/en-us/`, then retry. If the installer was not staged, install the current Microsoft Visual C++ x64 redistributable from Microsoft. No Unreal Editor or Visual Studio is needed to play a correctly packaged build.

The current package is a development beta candidate, not a signed or performance-certified release. The agreed minimum PC and frame-time target are pending in GitHub issue #7. If it fails to start, record the Windows version, CPU, GPU, RAM, graphics driver version, and any error text. Do not lower resolution or visual settings to hide a performance problem without an explicit player choice and visual review.

For developers, run `& .\unreal\Ridgefire\Tools\Build-WindowsShipping.ps1` from the repository root after installing UE 5.8.3 and the Windows C++ toolchain. Each run writes a timestamped, self-contained archive under `unreal/Ridgefire/Saved/Packages/`. The script builds, cooks, stages, packages, and checks for the launch executable; it does not publish or sign a release.
