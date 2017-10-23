cd "%~dp0\.."

mkdir build
cd build

cmake .. -G "Visual Studio 14 2015 Win64" ^
-DCMAKE_CONFIGURATION_TYPES:STRING="Release"

cmake --build "." --target "ALL_BUILD" --config "Release"

copy "Release\*.exe" "..\windows"
cd "..\windows"

color_flow.exe "..\examples\lmb-freiburg_flownet2_result.flo" ".\flow.png"
colortest.exe 10 "colors.png"

pause
