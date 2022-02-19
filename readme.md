# Prepare
Download CEF from [Chromium Embedded Framework (CEF) Automated Builds](https://cef-builds.spotifycdn.com/index.html) 
and extract it on a subfolder renaming it in minimal way according to the version:

 * `Cef_Win64`
 * `Cef_Win32`
 * `Cef_Linux64`
 * `Cef_Liuxn32`
 * `Cef_Mac64`
 * `Cef_Win64ARM`
 * `Cef_Linux64ARM`
 * `Cef_Linux32ARM`
 * `Cef_Mac64ARM`

It is ok the **Minimal Distribution**
Use CMake to build the wrapper, setting where to build in a folder called "build"

Remember to copy with the executable all folder Release and Resource

