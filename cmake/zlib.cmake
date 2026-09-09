# Stock source version identified in the game; not a binary/ABI replacement.
include(FetchContent)
FetchContent_Declare(zlib121
  URL https://zlib.net/fossils/zlib-1.2.1.tar.gz
  URL_HASH SHA256=94ded52040ee9dd1c70cc3b9f01da283803c28c1194a5a40659e8cf7545990e3
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(zlib121)
# Match the identified object families. No gz-file API, inflateBack, programs,
# assembler alternatives, generated tables or upstream test suite.
set(zlib121_units adler32 compress crc32 deflate inffast inflate inftrees trees uncompr zutil)
set(zlib121_sources)
foreach(unit IN LISTS zlib121_units)
  list(APPEND zlib121_sources "${zlib121_SOURCE_DIR}/${unit}.c")
endforeach()
add_library(bsp_zlib121 STATIC ${zlib121_sources})
target_include_directories(bsp_zlib121 PUBLIC "${zlib121_SOURCE_DIR}")
target_compile_definitions(bsp_zlib121 PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_options(bsp_zlib121 PRIVATE /W0)
