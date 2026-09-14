include_guard(GLOBAL)

# The metadata object must be a real linker input. Archiving a metadata-only
# member would not establish extraction or SafeSEH table membership.
enable_language(ASM_MASM)
add_library(bsp_native_crt_seh4_safe_handler OBJECT
  "${CMAKE_SOURCE_DIR}/src/native_crt_seh4_safe_handler.asm")
target_compile_options(bsp_native_crt_seh4_safe_handler PRIVATE /safeseh /WX)

# The first deferred callback queues attachment after every startup callback,
# including bsp_game creation, regardless of registry include ordering.
function(bsp_attach_native_crt_seh4_safe_handler)
  cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_game
    PRIVATE "$<TARGET_OBJECTS:bsp_native_crt_seh4_safe_handler>")
  # .sxdata alone does not extract the external handler's archive member.
  # Root the exact real C++ entry as well as supplying its registration object.
  cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_link_options bsp_game
    PRIVATE "/INCLUDE:?handle_native_crt_seh4_nested_unwind_00c0dc54@bsp@@YA?AW4_EXCEPTION_DISPOSITION@@PAU_EXCEPTION_RECORD@@PAXPAU_CONTEXT@@1@Z")
endfunction()
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}"
  CALL bsp_attach_native_crt_seh4_safe_handler)
