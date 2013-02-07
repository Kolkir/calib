mkdir ./fltk
cd ./fltk
cmake -DOPTION_USE_SYSTEM_LIBJPEG=FALSE -DOPTION_USE_SYSTEM_ZLIB=FALSE -DOPTION_USE_SYSTEM_LIBPNG=FALSE ../../src/external/fltk
cd ..
mkdir ./calib
cd ./calib
cmake ../../src