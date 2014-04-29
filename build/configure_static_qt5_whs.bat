c:\"Program Files (x86)\Microsoft DirectX SDK (June 2010)\Utilities\bin\dx_setenv.cmd"
SET CL=/MP
configure.bat -debug-and-release -opensource -platform win32-msvc2010 -static -ltcg -no-accessibility -no-openvg -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -no-qml-debug -nomake examples -nomake tests -no-icu -angle
rem configure.bat -debug-and-release -opensource -platform win32-msvc2010 -static -no-ltcg -no-accessibility -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -nomake examples -nomake tests -no-nis -angle -no-directwrite -no-qml-debug -no-icu -MP