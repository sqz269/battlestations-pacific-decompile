# Native camera frame command

Addresses: 00b71360

`execute_native_camera_frame_command_00b71360` reconstructs the complete normal
routine on actual camera, renderer, viewport and shared global storage. It uses
the complete native camera preparation, viewport binding and Clear providers.
The explicit context is a new source interface; it adds no native object field.

| Routine | Bounds inclusive | Original ABI | Coverage |
| --- | --- | --- | --- |
| `00B71360` | `00B71360..00B713C5`, 102 bytes | ECX actual camera; RET; no semantic return | complete |

The one enabled-byte read at camera `+17C` gates the whole command. If enabled,
it performs these current virtual dispatches against concrete renderer profile
`00D5F0A8`. The actual global `00F8D394` and current receiver profile are read
again before every call; a receiver changed by an earlier device callback is
used by the next dispatch.

| Call site | Native slot / concrete callee | Complete provider contract |
| --- | --- | --- |
| `00B7137B` | `+A0` / `00B285A0` | ECX renderer, stacked actual camera, RET4. Full preparation, including cache getters, clip state and ambient path. |
| `00B71392` | `+A4` / `00B26770` | ECX renderer, stacked borrowed viewport, RET4. Guarded viewport/scissor binding and attempted-call counter. |
| `00B713C2` | `+08` / `00B21430` | ECX renderer, six stacked arguments, RET18h. Complete existing Clear provider and its conditional guard/unwind schedule. |

The viewport field `+180` is read after the second receiver/profile read but
before its `+A4` slot read. It is passed as the same borrowed raw pointer, with
no retain, release, wrapper, state copy or default device callback.

After viewport binding, native order is FLD camera `+18C`, load stencil `+194`,
reload renderer and its Clear slot, push stencil, reserve the depth argument
word, FSTP that word, form color address `+190`, load flags `+188`, push flags,
null rectangles and zero count, then call Clear. The source preserves this
schedule, including the FLD/FSTP for zero flags. Clear itself independently
skips its guard, color/depth/device access and counter when flags are zero.
No FP environment reset or outer exception handler is introduced.

The context borrows the actual global address, the actual D5F0A8 profile words,
and fixed existing provider contexts. Each current renderer must carry the
native D5F0A8 tag. Current slots must remain B285A0/B26770/B21430; unsupported
tags or slots execute UD2 instead of substituting behavior. These checks add
integer instructions only. The valid native storage and provider-domain
requirements still apply. No novel descriptor projection is supported.

Producer evidence is camera constructor B71A80: B71ADD publishes the viewport;
B71C4B writes flags, B71C69 stencil, B71C6F enabled, B71C75 depth and B71C8D
color. This agrees with the established `NativeCameraTailStorage` layout and
its static offset checks. No duplicate raw camera type was introduced.

All four current xrefs were checked: B15E81/B161F0 inside B156C0 consume the
camera returned by B1BF40 through EAX to ECX; B4F2D6 inside B4F1C0 consumes
`[ESI+34]`; B1D9A0 inside B1D950 consumes EBP captured from context `+08`.
Every caller passes camera in ECX and supplies no stacked command argument.
The full B71360 listing establishes ESI as the saved camera throughout.

Fresh CLI batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, bridge 8089. All five complete spans (parent,
three providers and 168 profile bytes; 1,275 bytes total) match the installed
PE at `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
All four routines now have complete Ghidra bodies; no missing-function waiver
is used. Ghidra was read only; integration owns annotations and ledgers.

Validation results and reproducible fixture paths are recorded in
`reports/native_camera_frame_command.json`. The ignored fixture contains the
complete original bytes, generator, source, compiler script and run log.
It executes all 102 original parent bytes with exactly three declared absolute
global-address operand relocations. The reference profile relocates its three
callable slots to integer-only context/tail bridges into complete current
library descendants. Its three receiver vptrs point to that relocated table;
only those three pointer words are normalized in complete state comparisons.
Source receivers use the native D5F0A8 tag and unchanged native profile words.
Camera, renderer, viewport and current-global storage are otherwise the same
physical storage in each paired run. It does not execute the original
descendant machine bodies or their original EH registrations.

MSVC Win32 `/MD /W4 /WX /fp:strict` source compilation passed. The fresh
`scripts/build.ps1` build, both existing CTests and seven report call rows
passed; the verifier reports the three vtable calls as resolved indirects.
The replay compiles only `probe.cpp` against current `bsp_core`, `bsp_lua511`
and `bsp_zlib121` libraries, with `/MANIFEST:EMBED`; it compiles no production
source or saved object separately into the probe.

Fifteen original/source pairs pass: finite Clear/readback, flags-zero signaling
NaN and denormal under CW007F/027F/067F/0E7F with MXCSR1F80/9FC0; disabled
command with inaccessible context; unmasked-invalid CW027E; and C++ exception
after actual HAL Clear. Every normal pair retains the incoming x87 value,
control word, exact resulting status and MXCSR. HAL calls set MXCSR precision
status in both paths. The flags-zero cases retain IE or DE; unmasked signaling
NaN faults at FSTP [ESP] before Clear, with matching fault status/control/tag
and live x87 data. A thrown Clear callback drains its actual guard and skips
the attempted counter, without inventing parent cleanup. Real HAL readback
observes RGB1256AB in the selected viewport. The full event/storage traces
and raw FP frames are retained with hashes in the fixture manifest.

The fixture uses an isolated hidden real D3D9 HAL device, forwarding COM
observers and actual Win32 critical sections. Its cache-warm camera makes the
FP/dispatch boundary focused; cold frustum/user-plane/ambient branches retain
their providers' existing evidence. Source-interface, original-parent fixture,
real HAL, and bounded exception evidence do not establish drop-in binary ABI,
full descendant EH/SEH equivalence, a gameplay frame or visual game parity.
