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
This dependency suppresses its upstream compiler warnings; reconstructed C++
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

## zlib 1.2.1

`cmake/zlib.cmake` fetches the unmodified
[official zlib 1.2.1 archive](https://zlib.net/fossils/zlib-1.2.1.tar.gz)
into ignored build storage. The 345833-byte archive is pinned to SHA-256
`94ded52040ee9dd1c70cc3b9f01da283803c28c1194a5a40659e8cf7545990e3`,
calculated from the official download. The upstream license notice is copied
to [zlib121-LICENSE.txt](zlib121-LICENSE.txt); source notices remain unchanged.

Static target `bsp_zlib121`, linked into `bsp_core`, builds the ten C translation
units identified in the game's library inventory. The unused gz-file API,
inflateBack, sample programs and upstream test suite are excluded. Upstream
compiler warnings are suppressed only for this target; reconstructed C++
keeps its normal warning settings. First configuration requires the archive;
subsequent builds use the CMake dependency cache. No system zlib is installed.

The Win32 build and two existing CTests pass. The full D3D9 probe passes but
does not exercise zlib. Matching the source version does not establish native
ABI, differential behavior or archive runtime equivalence. See
[ZLIB_DEPENDENCY.md](../docs/ZLIB_DEPENDENCY.md) for source configuration and
the separate game streaming-wrapper evidence.
