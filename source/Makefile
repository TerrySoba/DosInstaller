objects_install = install.o blue_menu.o
coptions  = -ml -oneatx -xs -zp4 -0 -ad
unyar     = unyar.exe
install = install.exe

BUILD_DATE=`date -R`

all: $(install)

$(install): _$(install)
	upx -9 --ultra-brute -f --8086 -o$(install) _$(install)

%.o : %.cpp
	wpp $(coptions) $<

-include *.d

FORCE:

main.o: version.h

version.h: FORCE
	echo \#define BUILD_DATE \"$(BUILD_DATE)\" > version.h

_$(install): $(objects_install)
	echo "NAME   $@" > install.lnk
	echo "SYSTEM DOS" >> install.lnk
	echo "FILE   {$(objects_install)}" >> install.lnk
	wlink  @install.lnk
