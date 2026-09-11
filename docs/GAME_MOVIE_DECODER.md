# Bink movie decoder wrapper
Addresses: `00a4c660`, `00a4c700`, `00a4c710`, `00a4c720`, `00a4c860`,
`00a4c980`, `00a4ca40`, `00a4cb50`, `00a4cb60`, `00a4cb90`, `00a4cbc0`,
`00a4cbd0`, `00a4cc00`, `00a4cc10`, `00a4cc70`, `00a4cd30`, `00a4cd70`,
`00a4cee0`, `00a4cf30`, `00a4cfb0`, `00a4d140`, `00a4d1b0`.
Evidence dependencies: `00737730`, `00a4c770`, `00a4c8b0`, `00a4c920`,
`00a4d040`, `00a4d0f0`, `00bdd600`, `00bf7420`, `00d24d98`, `00d24de0`,
`00d24de8`, `00d24e40`, `00d24e48`, `00ce23a4`..`00ce23d4`.

`movie_decoder.hpp` and `movie_decoder.cpp` reconstruct the game's complete
38h Bink file/texture decoder, its shared 28h state, and normal control flow.
The code calls supplied typed imports for the shipped third-party Bink DLL.
It does not implement the codec. The host must provide real VFS, string pool,
renderer resources and reference-counted surface operations.

## Object and widget binding

`00737730` allocates 38h, calls `00a4c660` with the hidden construction flag 1,
and returns the shared base at allocation+10h. `MovieWidgetState::decoder`
therefore corresponds to `&NativeMovieDecoder::state`. The allocation and state
must not be interchanged when binding the widget host.

| Allocation offset | Native field |
| --- | --- |
| 00h | file vbtable 00D24DE8, shared offset 10h |
| 04h | texture vbtable 00D24DE0, shared offset 0Ch |
| 08h | texture resource pointer |
| 0Ch | vtordisp; final complete object value 0 |
| 10h | shared state vtable 00D24D98 |

Shared offsets are vtable 0, handle 4, running 8, completed 9, loop Ah, an
unresolved zero-initialized byte Bh, volume Ch, target volume 10h, fade delay
14h, time 18h, seconds per frame 1Ch and native filename string 20h/24h.
The Win32 build asserts the layout. `00a4cee0` sets volume to 1 and all other
state scalars to zero. The complete constructor preserves the observed base
construction order and final vtable/vtordisp. Only the complete-object flag-1
path is implemented; arbitrary further-derived virtual-base layouts are not.
The recorded table words are evidence IDs rather than callable addresses.

`00a4c860` performs the complete destructor chain without operator delete:
most-derived table, texture-base table and release, file-base table, shared-base
table, close and filename release. Filename destruction retains the header,
matching the native destructor. It must not be called again on destroyed state.

## Open, close and controls

`00a4ca40` receives three stack arguments and uses `RET 0Ch`; decompilation
incorrectly shows one. Concrete ECX is allocation+8h, adjusted from shared
state by `00a4c800`. The supplied byte/int are unused. It calls base open with
`(filename,1,0)`, copies the path, runs `00bdd600` on singleton 0109CEEC, selects
sound track 0, calls BinkOpen(path,4000h), stores the handle, releases the copied
path, and returns whether the handle is nonnull. Path resolution's boolean
result is ignored. The original, unresolved filename is retained for logging.

Base open resets time and completed **before** closing an existing handle.
Close marks completed and then closes/clears the handle. Consequently replacing
an open handle leaves completed set; open does not reset running, volume, loop,
or cached seconds per frame. These omissions are preserved. Base open always
returns false. Close logs even without an open handle and retains texture and
filename. The GUI layer is responsible for the subsequent prepare/start order.

Completion and loop setters preserve the entire input byte. Running false to
nonzero seeks frame 0 and resets time, then stores the supplied byte. Other
running transitions only store the byte; they do not call BinkPause. The pause
method calls BinkPause only with a handle and sets a 0.5-second fade delay when
the input byte is zero. Seeking clears completed even without a handle; with
a handle it calls BinkGoto(frame,0), initializes frame duration if zero, and
stores frame times duration. It does not decode immediately.

## Volume and advancement

Immediate volume sets both current and target values, then invokes concrete
volume application. Target-only assignment changes no other field. Concrete
application clamps only the Bink argument to [0,1], multiplies by the exact
double constants 0.707106769084930419921875 and 32768, truncates via native
00BF7420 and calls BinkSetVolume(handle,0,value). It stores the original input
after that call; there is no null-handle guard. Runtime 0109EEA4 selects SSE2
or x87 truncation. Their NaN integer results differ and are selected through
the explicit host flag. Full x87 status/control-word exception behavior and
extended-intermediate rounding parity are not claimed by these typed APIs.

Frame duration initializes lazily from BinkGetRealtime(handle,out,1):
unsigned frame-rate divisor / unsigned frame rate, followed by one float
store. There is no divide-by-zero guard and cached duration survives open.

Update returns false without a handle. Otherwise it initializes duration and
calls BinkWait even when stopped. Only ready and running decoders proceed.
The realtime frame number FFFFFFFFh becomes zero; time is frame times duration,
with float stores before delta computation. Every thirtieth frame logs. If
current and target volume differ, positive delay decreases by delta; otherwise
delay becomes zero and existing `0042ac60` steps volume by twice delta. Both
arms apply the current volume to Bink. At the last frame, nonlooping playback
marks completed and returns false before decoding; looping calls BinkGoto(0,0)
then decodes. Every other return is true. A prior completed byte alone does not
block this body.

## Texture preparation and decoding

Prepare releases any existing texture, clears its field, then with an open
handle calls renderer virtual +88h with `(width,height,1,16h,1)`. Returned null
is retained. It does not decode. Decode always calls BinkDoFrame. With a texture,
it acquires surface `(0,0)` via texture +30h, locks via surface +2Ch into an
8h `{pitch,pixels}` output initialized to zero, calls BinkCopyToBuffer with
`(handle,pixels,pitch,height,0,0,3)`, invokes texture +38h with zero, and releases
the temporary surface. HRESULT/Bink results are ignored in the original and
reconstruction. Native resource release is InterlockedDecrement(+4), followed
by virtual +0 only when the resulting count is zero. After optional upload,
decode calls BinkNextFrame only when frame number differs from frame count.
No null surface or handle success fallback is introduced.

## Evidence and validation boundaries

Read-only `bsp.py ghidra` queries verified the configured `bsp` project,
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 language and image
base before their batches. Pseudocode was checked against assembly for hidden
arguments, virtual inheritance, x87 expressions, conversion and upload order.
Missing functions were decoded from the configured disk PE after live byte
checks; final instruction starts, sizes and exclusive ends are recorded in
`reports/game_movie_decoder.json`. The primary integrator owns definitions,
annotations and project save. No worker Ghidra mutations were made.

The installed executable import table and installed `binkw32.dll` PE exports
agree on the eleven used decorated entrypoints. Typed `__stdcall` signatures
capture argument widths and stack bytes. Only used handle fields at 0/4/8/Ch
and the 38h realtime output extent are modeled. Bink allocates the full handle;
the prefix type must never be used to allocate one. Return types for ignored
import results follow the normal Bink contract and are not game-tested here.

These are native data layouts with new C++ entrypoint signatures, not drop-in
ABI replacements. Native SEH/unwind handlers, arbitrary subclasses and vtable
overrides, renderer/VFS backing, floating exception-state parity, codec output
and game/visual playback remain separate validation boundaries. Build, seed
and existing-check results are recorded in the report; no new permanent test
framework or codec implementation is added.

`scripts/build.ps1` passed the MSVC Win32 Release build and both existing CTest
checks (`reconstructed_math`, `native_math_differential`). Seed verification
passed the existing seed ranges. These checks establish compilation and shared
math regressions; they do not exercise Bink movie decoding or renderer output.
