jusbpmp
=======

jusbpmp - fork of the Java library for USB portable devices https://code.google.com/p/jusbpmp/

Notes
=====

The JNI files can be found in %JAVA_HOME%\include and %JAVA_HOME%\include\win32

Additional libraries needed to build the DLL are found in the Windows Media Format SDK, which due to the delights of the Microsoft site can be found at http://download.microsoft.com/download/a/c/3/ac367925-39e7-4451-a175-a224f94fbdce/wmformat11sdk.exe

The ANT script uses msbuild (part of the .NET framework) and VCBuild (part of Microsoft Visual C++) to build, these need to be on your PATH for it to work.

On my system, msbuild.exe was in C:\Windows\Microsoft.NET\Framework64\v4.0.30319 and vcbuild in C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin