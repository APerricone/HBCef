# Prepare
Download CEF from [Chromium Embedded Framework (CEF) Automated Builds](https://cef-builds.spotifycdn.com/index.html) 
and extract it on a subfolder renaming it in minimal way according to the version:

 * `cef_win64`
 * `cef_win32`
 * `cef_linux64`
 * `cef_linux32`
 * `cef_mac64`
<!-- * `cef_win64arm`-->
<!-- * `cef_linux64arm`-->
<!-- * `cef_linux32arm`-->
<!-- * `cef_mac64arm` -->

It is ok the **Minimal Distribution**
Use CMake to build the wrapper, setting where to build in a folder called "build"

Remember to copy with the executable all folder Release and Resource, default output is test* folder


