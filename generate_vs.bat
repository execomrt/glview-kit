@echo off
pushd "%~dp0"
cmake --preset vs2022-x64 %*
set "result=%errorlevel%"
popd
exit /b %result%
