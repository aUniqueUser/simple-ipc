del server.obj
del server.pdb
del server.exe
del client.obj
del client.pdb
del client.exe

cl /EHsc example/client.cpp Advapi32.lib /Isrc/include /DWIN32
cl /EHsc example/server.cpp	Advapi32.lib /Isrc/include /DWIN32 