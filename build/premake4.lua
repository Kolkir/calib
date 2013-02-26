solution "LScanner"
    location "lscanner"
    configurations { "Debug", "Release" }
   
    project "LScanner"
        kind "WindowedApp"
        language "C++"
        includedirs {"../src/3rdparty/fltk"}
                     
        files { "../src/*.h", 
                "../src/*.cpp",
                "../src/gui.h", 
                "../src/gui.cxx", 
                "../src/imgproc/*.h", 
                "../src/imgproc/*.cpp",
                "../src/misc/*.h", 
                "../src/misc/*.cpp",                
                "../src/serial/*.h", 
                "../src/serial/*.cpp",                                
                "../src/vcapture/*.h", 
                "../src/vcapture/*.cpp"                
              }

        configuration "windows"
            links { "user32", "gdi32"}
            defines {"WIN32", "NOMINMAX", "_CRT_SECURE_NO_WARNINGS"}
            
        configuration {"Debug", "windows"}
            prebuildcommands {"cd ..\\..\\src \n ..\\build\\fltk\\bin\\Debug\\fluid.exe -c gui.fl"}
            buildoptions {"/wd4995"}
            libdirs {"./fltk/lib/Debug"}
            links {"fltkd","fltkimagesd","fltkjpegd","fltkpng","zlibd"}
            targetdir "bin/Debug"

        configuration {"Release", "windows"}
            prebuildcommands {"cd ..\\..\\src \n ..\\build\\fltk\\bin\\Release\\fluid.exe -c gui.fl"}
            buildoptions {"/wd4995"}
            libdirs {"./fltk/lib/Release"}
            links {"fltk","fltkimages","fltkjpeg","fltkpng","zlib"}        
            targetdir "bin/Release"
            
        configuration "linux"
            prebuildcommands {"cd ../../src \n ../build/fltk/bin/fluid -c gui.fl "}
            buildoptions {"-std=c++0x"}
            libdirs {"./fltk/lib"}
            links {"X11", "pthread", "Xinerama", "Xft", "Xext", "fltk", "fltk_gl"}
            
        configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols", "Unicode" }

        configuration "Release"
        defines { "NDEBUG" }
        flags { "OptimizeSpeed", "Unicode" }        
