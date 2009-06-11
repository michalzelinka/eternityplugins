#Modified by ur4ltz

all: clean plugins icons
icons: xstatus_icons qipstatus_icons tzer_icons

plugins:
	i686-pc-mingw32-gcc -c -DBUILD_DLL *.c UI/*.c changeinfo/*.c -I../../include -I/usr/i686-pc-mingw32/usr/include -I. -w -march=i686 -msse -O2 -pipe
	i686-pc-mingw32-windres -i resources.rc -o resources.o
	i686-pc-mingw32-gcc -shared -o bin/Plugins/icq.dll *.o -lcomctl32 -lws2_32 -lgdi32 -lcomdlg32 -Wl,-O1,-s
	upx -9 --compress-icons=0 icq.dll

xstatus_icons:
	i686-pc-mingw32-windres -i icons_pack/xstatus_icons.rc -o xstatus_icons.o
	i686-pc-mingw32-ld --nmagic -s -e 0  -o bin/Icons/xstatus_ICQ.dll xstatus_icons.o

qipstatus_icons:
	i686-pc-mingw32-windres -i icons_pack/qip_statuses/qip_statuses.rc -o qip_statuses.o
	i686-pc-mingw32-ld --nmagic -s -e 0 -o bin/Icons/qipstatus_icons.dll qip_statuses.o

tzer_icons:
	i686-pc-mingw32-windres -i icons_pack/tzer_icons/tzer_icons.rc -o tzer_icons.o
	i686-pc-mingw32-ld --nmagic -s -e 0  -o bin/Icons/tzer_icons.dll  tzer_icons.o

clean:
	rm *.o

clean-all: 
	rm *.o *.dll
	