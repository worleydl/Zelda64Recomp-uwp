cmake_minimum_required(VERSION 3.14)
include(FetchContent)

# Setup UWP platform
set(CMAKE_SYSTEM_NAME WindowsStore)
set(CMAKE_SYSTEM_VERSION 10.0.19041.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch shared UWP deps
FetchContent_Declare(UwpLibs
    GIT_REPOSITORY https://github.com/worleydl/uwp-dep.git
    GIT_TAG 30c7ae708e91015ccced8442c137b5be7aececac 
)
FetchContent_MakeAvailable(UwpLibs)

# Define project
project(z64recomp-uwp LANGUAGES CXX)
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${PROJECT_NAME})

# Config for main recomp build
set(RECOMP_DIR CACHE PATH "")

add_executable(${PROJECT_NAME} WIN32
	main.cpp
)

set(BinDeps
	${CMAKE_CURRENT_BINARY_DIR}/_deps/uwplibs-src/x64/bin/dxil.dll
	${CMAKE_CURRENT_BINARY_DIR}/_deps/uwplibs-src/x64/bin/dxcompiler.dll
	${CMAKE_CURRENT_BINARY_DIR}/_deps/uwplibs-src/x64/bin/libuwp.dll
	${CMAKE_CURRENT_BINARY_DIR}/_deps/uwplibs-src/x64/bin/SDL2.dll

	Package.appxmanifest
)
source_group("BinDeps" FILES ${BinDeps})
set_source_files_properties(${BinDeps} PROPERTIES
	VS_COPY_TO_OUT_DIR Always
	VS_DEPLOYMENT_CONTENT TRUE
	VS_DEPLOYMENT_LOCATION "."
)

set(PkgAssets
	Assets/LockScreenLogo.scale-200.png
	Assets/Square44x44Logo.targetsize-24_altform-unplated.png
	Assets/Square44x44Logo.scale-200.png
	Assets/Square150x150Logo.scale-200.png
	Assets/StoreLogo.png
)
source_group("PkgAssets" FILES ${PkgAssets})
set_source_files_properties(${PkgAssets} PROPERTIES
	#VS_COPY_TO_OUT_DIR Always
	VS_DEPLOYMENT_CONTENT TRUE
	VS_DEPLOYMENT_LOCATION "Assets"
)

# Setup data deployment
file(GLOB_RECURSE DATA_FILES
	RELATIVE ${CMAKE_SOURCE_DIR}/..
	${CMAKE_SOURCE_DIR}/../assets/*
)

foreach(ASSET ${DATA_FILES})
	set(FULL_PATH "${CMAKE_SOURCE_DIR}/../${ASSET}")

	get_filename_component(DEST_DIR ${ASSET} DIRECTORY)

	set_source_files_properties("${FULL_PATH}" PROPERTIES
		VS_COPY_TO_OUT_DIR Always
		VS_DEPLOYMENT_CONTENT TRUE
		VS_DEPLOYMENT_LOCATION "${DEST_DIR}" # preserves folder structure
	)

	target_sources(${PROJECT_NAME} PRIVATE "${FULL_PATH}")
	source_group("BinData" FILES "${FULL_PATH}")
endforeach()

target_sources(${PROJECT_NAME} PRIVATE
	${BinDeps}
	${PkgAssets}
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/_deps/uwplibs-src/x64/include
)

set(BinLibs
	${CMAKE_CURRENT_BINARY_DIR}/_deps/uwplibs-src/x64/lib/freetype.lib
	${CMAKE_CURRENT_BINARY_DIR}/_deps/uwplibs-src/x64/lib/libuwp.lib
	${CMAKE_CURRENT_BINARY_DIR}/_deps/uwplibs-src/x64/lib/SDL2.lib

	${RECOMP_DIR}/lib/lunasvg/lunasvg.lib

	${RECOMP_DIR}/lib/N64ModernRuntime/librecomp/librecomp.lib
	${RECOMP_DIR}/lib/N64ModernRuntime/librecomp/miniz/miniz.lib
	${RECOMP_DIR}/lib/N64ModernRuntime/librecomp/N64Recomp/lib/fmt/fmt.lib # Note: Needs fmtd for debug
	${RECOMP_DIR}/lib/N64ModernRuntime/librecomp/N64Recomp/LiveRecomp.lib
	${RECOMP_DIR}/lib/N64ModernRuntime/librecomp/N64Recomp/N64Recomp.lib
	${RECOMP_DIR}/lib/N64ModernRuntime/librecomp/N64Recomp/rabbitizer.lib
	${RECOMP_DIR}/lib/N64ModernRuntime/librecomp/N64Recomp/SymbolLists.lib
	${RECOMP_DIR}/lib/N64ModernRuntime/ultramodern/ultramodern.lib;

	${RECOMP_DIR}/lib/RmlUi/Source/Core/rmlui.lib;
	${RECOMP_DIR}/lib/RmlUi/Source/Debugger/rmlui_debugger.lib;

	${RECOMP_DIR}/PatchesLib.lib
	${RECOMP_DIR}/RecompiledFuncs.lib

	${RECOMP_DIR}/rt64/src/contrib/nativefiledialog-extended/src/nfd.lib
	${RECOMP_DIR}/rt64/src/contrib/re-spirv/re-spirv.lib
	${RECOMP_DIR}/rt64/src/contrib/zstd/build/cmake/lib/zstd_static.lib
	${RECOMP_DIR}/rt64/rt64.lib

	${RECOMP_DIR}/../lib/rt64/src/contrib/dxc/lib/x64/dxcompiler.lib

	${RECOMP_DIR}/Zelda64Recompiled.lib

	d3d12.lib
	dxgi.lib
	dxguid.lib
	WindowsApp.lib
	OneCore.lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${BinLibs})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION 10.0.19041.0)
