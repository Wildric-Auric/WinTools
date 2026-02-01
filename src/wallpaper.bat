@echo off
set "wp_dir=C:/Wallpapers/"
pushd %~dp0
call _wallpaper %wp_dir% %* 
popd
