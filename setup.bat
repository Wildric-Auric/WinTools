@echo off
if not exist "bin/"              (mkdir bin)
if not exist "thirdparty/"       (mkdir thirdparty)
if not exist "json.h"            (git clone "https://github.com/sheredom/json.h")
cp json.h/json.h thirdparty/json.h
