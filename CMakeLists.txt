cmake_minimum_required(VERSION 3.10)

project(backslash_n CXX)

# The demo is C++ written for BeOS R5 / gcc 2.95; Haiku ships a modern gcc and
# the code relies on type punning in the polygon sorter, hence the flags below.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Build type" FORCE)
endif()

set(DEMO_SOURCES
	sources/GameWindow.cpp
	sources/WarIFFChunk.cpp
	sources/dApp.cpp
	sources/dBitmap.cpp
	sources/dClut1.cpp
	sources/dClut2.cpp
	sources/dContourObject.cpp
	sources/dDot.cpp
	sources/dFace.cpp
	sources/dLine.cpp
	sources/dMaterial.cpp
	sources/dObject.cpp
	sources/dPalette.cpp
	sources/dPart.cpp
	sources/dPart3DScene.cpp
	sources/dPart3DScene2.cpp
	sources/dPart3DSceneDist2.cpp
	sources/dPart3DSceneDots.cpp
	sources/dPart3DSceneSpin2.cpp
	sources/dPart3DSceneTwist.cpp
	sources/dPolygon.cpp
	sources/dScreen.cpp
	sources/dVertex.cpp
	sources/stsp.cpp
)

# The SawTeeth replayer the demo plays its tune with (see st/song.cpp).
set(SAWTEETH_SOURCES
	st/insply.cpp
	st/player.cpp
	st/song.cpp
	st/txt.cpp
	st/wave.cpp
)

add_executable(backslash_n ${DEMO_SOURCES} ${SAWTEETH_SOURCES})

target_include_directories(backslash_n PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/headers
	${CMAKE_CURRENT_SOURCE_DIR}/st
)

target_compile_options(backslash_n PRIVATE
	-Wall
	-Wno-multichar
	-fno-strict-aliasing
)

target_link_libraries(backslash_n
	be
	game
	media
	translation
)
