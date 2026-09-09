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

## D3DX SDK headers

cmake/d3dx.cmake fetches [Microsoft.DXSDK.D3DX9.29.952.8](https://www.nuget.org/packages/Microsoft.DXSDK.D3DX/9.29.952.8)
into ignored build storage, pinned to SHA256
ead0906ae8a26c18a7525da7490127a2110f7c58f18293738283e30e97c6ea4b.
Only its official headers are used as external system includes. The API
implementation is dynamically loaded from installed System32 D3DX9_40.dll;
the package DLLs are not copied into this repository or the output directory.
Package terms remain in its LICENSE.txt; see the accompanying local copy
DXSDK-D3DX-LICENSE.txt. No claim of matching D3DX compiler versions is made.
