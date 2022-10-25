
.PHONY:
all: executable

withjadel:
	(cd W:/jadel2 && make clean && make)
	cl /c /Zi /EHsc /Iinclude /IW:\jadel2\export\include /std:c++latest .\\src\\*.cpp /Foobj\ 
	cl /Zi obj\\*.obj W:\jadel2\export\lib\jadelmain.lib W:\jadel2\export\lib\jadel.lib /Febin\rlike.exe /link /SUBSYSTEM:WINDOWS
	copy "W:\jadel2\export\lib\jadel.dll" bin\jadel.dll

executable: obj\\*.obj
	cl /Zi /O2 obj\\*.obj W:\jadel2\export\lib\jadelmain.lib W:\jadel2\export\lib\jadel.lib /Febin\rlike.exe /link /SUBSYSTEM:WINDOWS

obj\\*.obj:
	cl /c /Zi /O2 /EHsc /Iinclude /IW:\jadel2\export\include /std:c++latest src\\*.cpp /Foobj\ 

.PHONY:
clean:
	del obj\\*.obj bin\\*.exe