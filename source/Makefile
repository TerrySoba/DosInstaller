objects  = main.o yar_decompressor.o decode.o checksum.o
coptions = -ml -oneatx -xs -zp4 -0 -ad
dest     = unyar.exe

BUILD_DATE=`date -R`

all: $(dest)

$(dest): _$(dest)
	upx -9 --ultra-brute -f --8086 -o$(dest) _$(dest)

%.o : %.cpp
	wpp $(coptions) $<

-include *.d

FORCE:

main.o: version.h

version.h: FORCE
	echo \#define BUILD_DATE \"$(BUILD_DATE)\" > version.h

_$(dest): $(objects)
	echo "NAME   $@" > game.lnk
	echo "SYSTEM DOS" >> game.lnk
	echo "FILE   {$(objects)}" >> game.lnk
	wlink  @game.lnk
