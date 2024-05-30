@echo off
if "%~1"=="" (
    echo Usage: %~nx0 proto_file
    exit /b
)
echo "%~nx1"
protoc --cpp_out=. "%~nx1"
echo "done"
pause