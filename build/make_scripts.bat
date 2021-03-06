mkdir fltk
cd fltk
cmake -DOPTION_USE_SYSTEM_LIBJPEG=FALSE -DOPTION_USE_SYSTEM_ZLIB=FALSE -DOPTION_USE_SYSTEM_LIBPNG=FALSE ../../src/3rdparty/fltk
cd ..

mkdir opencv
cd opencv
cmake -DBUILD_SHARED_LIBS=FALSE -DBUILD_WITH_STATIC_CRT=FALSE -DWITH_CUDA=FALSE ../../src/3rdparty/opencv
cd ..

premake4.exe vs2010
PAUSE