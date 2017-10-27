cd "%~dp0.."

rmdir /S /Q build
mkdir build
cd build

cmake .. -G "Visual Studio 14 2015 Win64" ^
-DCMAKE_CONFIGURATION_TYPES:STRING="Release;Debug"

cmake --build "." --target "ALL_BUILD" --config "Release"

copy "Release\colorflow.exe" "..\windows"
cd "..\windows"

colorflow "..\examples\lmb-freiburg_flownet2_result.flo"
colorflow -c 20 351

pause
