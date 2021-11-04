cls

cl.exe /c /EHsc /I"C:\glew\include" /I"include" OGLPP.cpp

rc.exe RESOURCES.rc

link.exe /LIBPATH:"C:\glew\lib\Release\x64" OGLPP.obj RESOURCES.res 

OGLPP.exe



