

CC=cl
ODIR=obj
IDIR=include W:/jadel/export/include
_DEPS=include
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))
CFLAGS=/I$(IDIR)
_OBJ=*.obj
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

.PHONY:
all: rlike

withjadel:
	(cd W:/jadel2 && make clean && make)
	copy W:\jadel2\export\lib\jadel.dll bin\jadel.dll
	$(CC) /c /Zi /EHsc /Iinclude /IW:/jadel2/export/include /std:c++latest ./src/*.cpp /Foobj/ 
	$(CC) /Zi obj/*.obj W:/jadel2/export/lib/jadelmain.lib W:/jadel2/export/lib/jadel.lib /Febin/rlike.exe /link /SUBSYSTEM:WINDOWS

rlike: $(OBJ)
	$(CC) /Zi $^ W:/jadel2/export/lib/jadelmain.lib W:/jadel2/export/lib/jadel.lib /Febin/$@ /link /SUBSYSTEM:WINDOWS

$(ODIR)/%.obj: src/%.cpp
	$(CC)  /c /Zi /EHsc /Iinclude /IW:/jadel2/export/include /std:c++latest $^ /Foobj/

copyjadel:
	$(CC)  /c /Zi /EHsc /Iinclude /IW:/jadel2/export/include /std:c++latest ./src/*.cpp /Foobj/ 
	$(CC)  /Zi obj/*.obj W:/jadel2/export/lib/jadelmain.lib W:/jadel2/export/lib/jadel.lib /Febin/rlike.exe /link /SUBSYSTEM:WINDOWS
	copy W:\jadel2\export\lib\jadel.dll bin\jadel.dll

.PHONY:
clean:
	del obj\\*.obj bin\\*.exe