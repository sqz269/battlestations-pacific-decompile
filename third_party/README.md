# Lua runtime dependency

The host shader-script adapter builds stock Lua5.1.1 core, auxiliary API and
base library. Source is fetched into ignored build storage by cmake/lua.cmake:
https://www.lua.org/ftp/lua-5.1.1.tar.gz

The archive SHA256 is pinned to
c5daeed0a75d8e4dd2328b7c7a69888247868154acbda69110e97d4a6e17d1f0,
as listed at https://www.lua.org/ftp/. The unmodified upstream license is
lua511-COPYRIGHT.txt. Preserve it when distributing binaries containing Lua.

First configuration needs access to the archive; subsequent builds use the
CMake download cache. Lua is compiled as C with its upstream code unchanged.
Only this dependency suppresses upstream compiler warnings; reconstructed C++
retains /W4 /WX /fp:strict. The game contains a Lua5.1.1 version label, but
stock source compatibility does not prove identical native patches, ABI,
allocator behavior, hash iteration or floating-point execution.
