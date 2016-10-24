setlocal
mkdir build
pushd build
cmake ^
	-G "Visual Studio 14 2015 Win64" ^
    -DOpenCV_DIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}/../../opencv/build ^
    -DOpenCV_FOUND:BOOL=TRUE ^
    -DOPENCV_FOUND:BOOL=TRUE ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_INSTALL_PREFIX=../dist .. ^
    -DCMAKE_VERBOSE_MAKEFILE=ON
msbuild /nologo /verbosity:minimal /clp:NoSummary mf_test.vcxproj
set PATH=%PATH%;C:/Users/Administrator/Desktop/opencv/build/x64/vc14/bin
Debug\mf_test.exe %1
popd

