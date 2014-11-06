"e:\devel\Microsoft DirectX SDK (June 2010)\Utilities\bin\dx_setenv.cmd"
set PATH=e:\devel\Git\bin;%PATH%
set PATH=e:\devel\strawberry-perl\perl\bin;%PATH%
set PATH=e:\devel\qt\5.3.2\qtbase\bin;%PATH%
set PATH=e:\devel\qt\5.3.2\gnuwin32\bin;%PATH%
set PATH=e:\devel\Python34;%PATH%
SET CL=/MP
configure.bat -debug-and-release -opensource -platform win32-msvc2013 -static -ltcg -no-accessibility -no-openvg -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -no-qml-debug -nomake examples -nomake tests -no-icu -angle
rem configure.bat -debug-and-release -opensource -platform win32-msvc2013 -static -no-ltcg -no-accessibility -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -nomake examples -nomake tests -no-nis -angle -no-directwrite -no-qml-debug -no-icu -MP