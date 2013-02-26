if [ ! -d "./fltk" ]; then
    mkdir ./fltk
fi
cd ./fltk
cmake -DOPTION_USE_SYSTEM_LIBJPEG=FALSE -DOPTION_USE_SYSTEM_ZLIB=FALSE -DOPTION_USE_SYSTEM_LIBPNG=FALSE ../../src/3rdparty/fltk
cd ..
if [ ! -d "./dslrsim" ]; then
    mkdir ./dslrsim
fi
./premake4 codeblocks
