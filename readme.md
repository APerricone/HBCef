# HBCef
HBCef is a library to use a subset of [CEF library](https://bitbucket.org/chromiumembedded/cef/src/master/) (Chromium Embedded Framework) functionability with harbour, to allow developer to create desktop application using web technologies.
It is inspired by [Electron](https://www.electronjs.org/)

> It's not a wrapper for CEF

Some current feature are
 * Create browser window
 
Some planned features are 
 * Add JS functions  to pages
 * Use of a custom scheme to serve internal pages
 * Use of CEF platform's indipendent dialog windows

# Use
Download CEF from [Chromium Embedded Framework (CEF) Automated Builds](https://cef-builds.spotifycdn.com/index.html) 
and extract it on a subfolder renaming it in short way according to the version:

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

Copy with the executable all files inside Release and Resource folders, default output is test* folder

Use CMake to build the wrapper, setting where to build in a folder called "build"
