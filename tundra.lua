local CFiles = { ".c", ".h", ".cc" }
local ObjCFiles = { ".m" }
local linux_common = {
        Tools = { "gcc" },
        Env = {
                CCOPTS = {
                        "`pkg-config --cflags libffi`",

                },
                CPPDEFS = {
                        "LINUX",
                        "K8",
                        "PA_TIMING_FIX"
                },
                PROGOPTS= {
                        "`pkg-config --libs gl`",
                }
        },
        ReplaceEnv = {
                LD = "$(CXX)"
        }
}

Build {
        Configs = {
                Config {
                        Name = "linux-gcc",
                        DefaultOnHost = "linux",
                        Inherit = linux_common,
                },
                Config {
                        Name = "freebsd-gcc",
                        DefaultOnHost = "freebsd",
                        Inherit = linux_common,
                        Env = {
                                CCOPTS = {
                                        "`pkg-config --cflags portaudio-2.0`",
                                },
                                PROGOPTS = {
                                        "`pkg-config --libs portaudio-2.0`",
                                },
                                CPPDEFS = {
                                        "FREEBSD"
                                },
                        },
                },
                Config {
                        Name = "macosx-gcc",
                        DefaultOnHost = "macosx",
                        Tools = { "clang-osx" },
                        Env = {
                                CPPDEFS = {
                                        "MACOSX",
                                        "K8"
                                },
                        },
                        ReplaceEnv = {
                                LD = "$(CXX)",
                        }
                },
                Config {
                        Name = "win64-msvc",
                        DefaultOnHost = "windows",
                        Tools = { "msvc-vs2008"; TargetPlatform = "x64" },
                        Defines = { "WIN32", "K8" }
                },
        },
        Units = function()
                require "tundra.syntax.glob"
                Program {
                        Name = "a.out",
                        Sources = {
                                Glob { Dir = ".", Extensions = CFiles },
                                {
                                        Glob { Dir = ".", Extensions = ObjCFiles } ;
                                        Config = "macosx-gcc"
                                }
                        },
                        Includes = {
                                "src",
                                "src/third-party/log4c"
                        },
                        Defines = {
                                'MAP_STL_IMPL',
                                'CSTR_MAP_STL_IMPL',
                                'KGO_DEFAULT=\\"sdlgl\\"',
                                'PAN_DEFAULT=\\"portaudio\\"',
                                'KNOS_RELEASE=-1',
                                'KNOS_BUILD=0',
                                'MMCMP_SUPPORT',
                                'CCGVERSION=""'
                        },
                        Libs = {
                                "ffi",
                                "portaudio",
                                "jpeg",
                                "png"
                        },
                        Frameworks = {
                                "OpenGL"
                        },
                        Env = {
                                CCOPTS = {
                                        "`sdl-config --cflags`",
                                        "`freetype-config --cflags`",
                                        { "-Os" ; Config = "*-gcc-release-*" },
                                        { "-Os" ; Config = "*-clang-release-*" },
                                        { "-O0 -g" ; Config = "*-*-debug-*" },
                                },
                                PROGOPTS = {
                                        "`sdl-config --libs`",
                                        "`freetype-config --libs`",
                                }
                        }
                }

                Default "a.out"
        end,
}
