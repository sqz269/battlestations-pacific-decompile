# Pinned zlib 1.2.1 dependency

`cmake/zlib.cmake` defines static target `bsp_zlib121` from the unmodified
[official zlib fossil archive](https://zlib.net/fossils/zlib-1.2.1.tar.gz).
The [official archive index](https://zlib.net/fossils/) lists that historical
release. The downloaded archive is 345833 bytes with SHA-256
`94ded52040ee9dd1c70cc3b9f01da283803c28c1194a5a40659e8cf7545990e3`.
This checksum was calculated from the retrieved official bytes, not attributed
to an upstream checksum manifest. CMake pins it with URL_HASH and extracts into
its dependency cache. There is no system installation or bundled executable.

The archive's `zlib.h` declares version 1.2.1, November 17, 2003, and
ZLIB_VERNUM 0x1210. Its unmodified introductory license notice is copied to
`third_party/zlib121-LICENSE.txt`. Upstream source remains unchanged and retains
the original notices. This is deliberately a historical reconstruction
dependency rather than a current general-purpose compression recommendation.

## Translation units and build boundary

The target follows `cmake/lua.cmake`: FetchContent, explicit C source list,
public include directory, private `_CRT_SECURE_NO_WARNINGS` and `/W0` only for
upstream code. CMakeLists includes this dependency and links `bsp_zlib121`
into `bsp_core`.

The source list is exactly the object families identified by
`reports/library_inventory/zlib_and_borrowed.json`:
adler32.c, compress.c, crc32.c, deflate.c, inffast.c, inflate.c, inftrees.c,
trees.c, uncompr.c, zutil.c. No gzio.c, infback.c, sample programs, assembler
alternatives or upstream test suite is included. Removing those unused units
does not alter the included upstream files.

The official win32/Makefile.msc uses CL with -MD/-O2 and no required special
LOC definitions. This CMake target inherits the repository's chosen MSVC
runtime/configuration instead of hardcoding a second runtime. It is intended
for the project's MSVC Win32 build, not a cross-platform build recipe.

No ZLIB_DLL or ZLIB_WINAPI is defined: public stock APIs use their ordinary
C calling convention, not the optimized register conventions observed in the
game. No DEBUG, FASTEST, NO_GZIP, MY_ZCALLOC, NOBYFOUR, ASMV or table-generation
override is introduced. Under MSVC's STDC path, crc32.c automatically selects
BYFOUR when it finds a four-byte integer; this agrees with the inventory's
8x256 CRC tables without forcing an internal macro. The normal built-in
tables and zcalloc/zcfree remain upstream code.

Version labels, stream size 38h and table/state-machine matches support using
this source version. They do not establish matching optimization, ABI,
instruction bytes, allocator failure behavior or runtime equivalence.

## Game raw-DEFLATE wrapper

The following separate game routines remain wrapper reconstruction work;
they are not replaced simply by linking stock zlib. Every Ghidra batch verified
project `bsp`, `/battlestationspacific.exe`, x86 LE base 00400000. Full raw spans
matched the installed PE:

| Start | Bytes | SHA-256 |
|---|---:|---|
| 00bbc1d0 | 334 | `e452dcd3c3ea85998071d0b82885e92bec5e174c9ad732225e2215355f091e86` |
| 00bbbf00 | 257 | `61dd0e75f92566b2d8ba514893c0faf7dbf29fc755d04fb48d2f67a93e931dbb` |
| 00bbbe10 | 50 | `44fbaedf220800085a67c0969e10d806d0dcc905b608efbd31ada85b74183c3d` |
| 00bbc320 | 186 | `fa39ded96a5501adf264511a8f061a57ca12fd12a49edcfbff017c412dca459a` |

00bbc1d0 has ECX wrapper and four stack arguments, RET 10h. The first is the
source stream; the second points to three DWORDs copied to wrapper +10/+14/+18:
source offset, compressed byte count, uncompressed byte count. The remaining
two stack slots are callee-cleaned but not consumed by this implementation;
buffer capacities are immediate constants, 4000h input and 10000h output. Do
not infer variable capacities from the misleading decompiler prototype.

The wrapper retains source at +C, allocates a 38h z_stream at +28, zeroes default
allocator/free/opaque and input fields, then calls inflateInit2_ at 00bc96a0 at
00bbc2d4 with ECX stream, EDX=FFFFFFF1h (-15), stack version `1.2.1` and 38h.
This is **raw DEFLATE**, not zlib/gzip-wrapped input. It increments source's
intrusive count, seeks source to the supplied offset, initializes position +24
to 0 and remaining counts +2C/+30 from the descriptor. The initialization
zlib status is ignored natively; allocation/source failures are not safely
projected by substituting unchecked host pointers.

00bbbf00 is ECX wrapper, no stack arguments, RET. It prepares the output
buffer as next_out/avail_out. If input is exhausted and compressed bytes
remain, it reads min(input-capacity, remaining) through source virtual +24,
subtracts the actual count and sets next_in/avail_in. Its zlib call at
00bbbfb6 uses **Z_FINISH(4)** only when compressed remaining is zero and
uncompressed remaining <= current avail_out; otherwise **Z_SYNC_FLUSH(2)**.
It subtracts the change in total_out from remaining uncompressed bytes and
updates the input cursor from next_in. It stops on Z_STREAM_END or any other
nonzero status, then publishes the produced output extent. This is not a
one-shot uncompress() call, and the pseudo-output omitted the flush argument.

00bbbe10 resets zlib, seeks the source back to descriptor offset, zeroes
position and restores remaining byte counts. It does not itself reset either
buffer's cursor/end pointers. A future seek implementation must inspect its
caller and neighboring buffer operations before claiming arbitrary rewind
parity. This fragment is not permission to discard stale buffered state by
guesswork.

00bbc320 destroys in order: release source +C; inflateEnd on +28; free z_stream;
free input backing and its 16-byte descriptor; free output backing and its
descriptor; base cleanup 00bd30f0. Raw continuation past erroneously no-return
`_free` was required; complete function returns at 00bbc3d9. Deleting wrapper
00bbc3e0 additionally frees the object when its flag requests deletion.

## Concrete next integration

The minimal next unit is the game stream adapter around stock raw inflate,
with typed retained source, explicit three-DWORD descriptor, exact buffer
sizes and flush decision. Before implementing complete read/seek semantics,
recover table 00d64400 virtual +1C = 00bbc060 (seek), +24 = 00bbc140 (read), and their
immediate output-drain helpers as actually called.
That work must resolve short reads, no-progress/error handling, buffered reset
and produced-byte reporting; exposing a whole decoded vector would bypass
those behaviors. MPKG entry parsing, XOR transform and provider lookup remain
the separate `ARCHIVE_PROVIDER_ENTRY.md` dependency.

## Validation

`./scripts/build.ps1` passes for all targets, including the integrated zlib
dependency, and both existing CTests pass. The full D3D9 probe also passes,
but does not exercise zlib. These checks establish build integration, not
zlib behavioral equivalence, native ABI compatibility or archive decoding.
No differential zlib fixture or archive runtime validation is claimed.

Ignored verification downloads are under `exports/bsp/zlib121/`; wrapper exports are under
`exports/bsp/owner_textures/zlib_wrapper/`.
