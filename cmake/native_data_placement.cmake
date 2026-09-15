# Raw native-data consumers require the original CF/D1/D5/D6 address bands.
# Place only the rebuilt Win32 game above that range, without executable ASLR
# randomizing its image, initial stack and heap into those bands. DLL policy and
# the suspended-child reservation/ownership guards remain unchanged.
# See docs/NATIVE_DATA_PLACEMENT_AC.md for the observed collisions and limits.
if(WIN32 AND MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    # The startup registry can schedule this include before deferred target creation.
    cmake_language(DEFER CALL target_link_options bsp_game PRIVATE
        "/BASE:0x10000000" "/DYNAMICBASE:NO" "/FIXED:NO"
        "/MAP:${CMAKE_BINARY_DIR}/bsp_game.map")
endif()
