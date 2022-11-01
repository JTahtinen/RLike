
.PHONY:
all: rlike

withjadel:
	(cd W:/jadel2 && make clean && make)
	cl /c /Zi /EHsc /Iinclude /IW:/jadel2/export/include /std:c++latest ./src/*.cpp /Foobj/ 
	cl /Zi obj/*.obj W:/jadel2/export/lib/jadelmain.lib W:/jadel2/export/lib/jadel.lib /Febin/rlike.exe /link /SUBSYSTEM:WINDOWS
	copy W:\jadel2\export\lib\jadel.dll bin\jadel.dll

rlike: obj/*.obj
	cl /Zi obj/*.obj W:/jadel2/export/lib/jadelmain.lib W:/jadel2/export/lib/jadel.lib /Febin/$@ /link /SUBSYSTEM:WINDOWS

obj/*.obj: src/*.cpp
	cl /c /Zi /EHsc /Iinclude /IW:/jadel2/export/include /std:c++latest $^ /Foobj/

copyjadel:
	cl /c /Zi /EHsc /Iinclude /IW:/jadel2/export/include /std:c++latest ./src/*.cpp /Foobj/ 
	cl /Zi obj/*.obj W:/jadel2/export/lib/jadelmain.lib W:/jadel2/export/lib/jadel.lib /Febin/rlike.exe /link /SUBSYSTEM:WINDOWS
	copy W:\jadel2\export\lib\jadel.dll bin\jadel.dll

.PHONY:
clean:
	del obj\\*.obj bin\\*.exe