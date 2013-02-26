solution "Calib"
    location "calib"
    configurations { "Debug", "Release" }
   
    project "Calib"
        kind "WindowedApp"
        language "C++"
        includedirs {"../src/3rdparty/fltk",
                     "../src/3rdparty/opencv/include",
                     "../src/3rdparty/opencv/modules/core/include",
                     "../src/3rdparty/opencv/modules/calib3d/include",
                     "../src/3rdparty/opencv/modules/features2d/include",
                     "../src/3rdparty/opencv/modules/flann/include",
                     "../src/3rdparty/opencv/modules/imgproc/include",
                     "../src/3rdparty/opencv/modules/highgui/include"}       
                     
        files { "../src/*.h", 
                "../src/*.cpp",
                "../src/gui.h", 
                "../src/gui.cxx",                                
              }

        configuration "windows"
            links { "user32", "gdi32"}
            defines {"WIN32", "NOMINMAX", "_CRT_SECURE_NO_WARNINGS"}
            
        configuration {"Debug", "windows"}
            prebuildcommands {"cd ..\\..\\src \n ..\\build\\fltk\\bin\\Debug\\fluid.exe -c gui.fl"}
            buildoptions {"/wd4995"}
            libdirs {"./fltk/lib/Debug", "./opencv/lib/Debug", "./opencv/3rdparty/lib/Debug"}
            links {"fltkd",
                   "fltkimagesd",
                   "fltkjpegd",
                   "fltkpng",
                   "zlibd",
                   "opencv_calib3d240d",
                   "opencv_core240d",
                   "opencv_highgui240d",
                   "opencv_imgproc240d",
                   "opencv_flann240d",
                   "opencv_features2d240d"}
            targetdir "bin/Debug"

        configuration {"Release", "windows"}
            prebuildcommands {"cd ..\\..\\src \n ..\\build\\fltk\\bin\\Release\\fluid.exe -c gui.fl"}
            buildoptions {"/wd4995"}
            libdirs {"./fltk/lib/Release", "./opencv/lib/Release", "./opencv/3rdparty/lib/Release"}
            links {"fltk",
                   "fltkimages",
                   "fltkjpeg",
                   "fltkpng",
                   "zlib",
                   "opencv_calib3d240",
                   "opencv_core240",
                   "opencv_highgui240",
                   "opencv_imgproc240",
                   "opencv_flann240",
                   "opencv_features2d240"}        
            targetdir "bin/Release"
            
        configuration "linux"
            prebuildcommands {"cd ../../src \n ../build/fltk/bin/fluid -c gui.fl "}
            buildoptions {"-std=c++0x"}
            libdirs {"./fltk/lib", "./opencv/lib/", "./opencv/3rdparty/lib"}
            links {"X11", "pthread", "Xinerama", "Xft", "Xext", "fltk", "fltk_gl"}
            
        configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols", "Unicode" }

        configuration "Release"
        defines { "NDEBUG" }
        flags { "OptimizeSpeed", "Unicode" }        
