local CFiles = { ".c", ".h", ".cc", ".m" }
Build {
	Configs = {
		Config {
			Name = "generic-gcc",
			DefaultOnHost = "linux",
			Tools = { "gcc" },
			Defines = { "LINUX", "K8" },
			ReplaceEnv = {
				LD = "$(CXX)"
			}
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
			Sources = { Glob { Dir = ".", Extensions = CFiles } },
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
					{ "-Os" ; Config = "*-clang-release-*" }
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
