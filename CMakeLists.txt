set (SkiaGUI_source_dir "${PROJECT_SOURCE_DIR}/SkiaGUI/src/")
file (GLOB_RECURSE SkiaGUI_source_files 
	"${SkiaGUI_source_dir}/*.cpp"
	"${SkiaGUI_source_dir}/*.c"
	)

add_library(SkiaGUI ${SkiaGUI_source_files})

target_include_directories(SkiaGUI PUBLIC 
	"/home/erik/Dev/skia/skia/third_party/externals/icu/source/common/"
	"/usr/include/SDL2/"
	"/home/erik/Dev/skia/skia/"
	"${PROJECT_SOURCE_DIR}/SkiaGUI/src/")