# Spine
The Spine launcher for Gothic modifications and other games.

## Building from source

To build Spine yourself to add bugfixes or new features, you need to set up the following:

* Download & install **Git**
* Download & install **Microsoft Visual Studio 2015** (newer versions might work as well, but the toolchain is only tested with VS 2015)
* Download & install **Qt 5.15** (newer version might work as well, but that's the one currently used for the Spine client)
* Download & install **CMake**
* Download & install **Perl** (e.g. Strawberry Perl) [required for building OpenSSL]
* Ensure either 7zip or WinRar are in PATH
* Set the environment variable **VS15** to the path of the **VSDevCmd.bat** (e.g. C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat)
* Run cmake (best would be using the GUI) and configure your build
** Set variable Qt5_BaseDir in CMake to the folder containing Qt (e.g. C:\Qt\Qt5.12.9\5.12.9)

That's it. Now you have a Visual Studio project to build Spine. It's a basic version without some features and will only work with the test server. If you want to test some server functionality you might need to contact us so we start the test server. At the moment it's disabled as there is no need for it.

### Building on Linux

Building on Linux using either g++ or clang++ is no problem (our build server builds the Linux configuration only). But we don't know whether the client will run. It won't work properly either way, as some functionality is disabled on Linux and the whole mechanism in Spine uses Windows specific features that won't work out of the box on Linux. We might add proper Linux and OS X support somewhen in the future though.
