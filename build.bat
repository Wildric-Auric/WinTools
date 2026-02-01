@echo off
clang src/cmdconf.c   -I thirdparty/ -o bin/cmdconf.exe 
clang src/wallpaper.c -I thirdparty/ -luser32 -o bin/_wallpaper.exe 
cp  src/wallpaper.bat bin/wallpaper.bat
