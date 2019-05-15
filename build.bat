set CURRENT_DIR=%~dp0
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
call bootstrap-vcpkg.bat
set VCPKG_ROOT=%CURRENT_DIR%\vcpkg
vcpkg.exe install --triplet x64-windows-static physfs libsquish
cd ..
mkdir build
cd build
cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static "-DCMAKE_TOOLCHAIN_FILE=%CURRENT_DIR%\vcpkg\scripts\buildsystems\vcpkg.cmake" -DBSZENLIB_DOWNLOAD_BSF_BINARIES=ON -G "Visual Studio 15 2017 Win64" ..
cmake --build . -j 8