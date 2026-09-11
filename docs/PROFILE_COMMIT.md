# Profile settings restore and content commit

The four recovered bodies live in `include/bsp/profile_commit.hpp` and
`src/profile_commit.cpp`. Names describe hypotheses, not recovered symbols.
These C++ interfaces are host projections; none is a binary replacement.

| Address | Proposed name | Native ABI | Recovered scope |
| --- | --- | --- | --- |
| `008d7a50` | `BSP_Settings_RestoreFromProfile` | callback on stack, `RET 4`; ECX unused | Storage/name gates, pending callback, queued or immediate read |
| `008d79a0` | `BSP_Settings_CompleteProfileRestore` | no arguments, `RET` | Settings reader, buffer/archive/reader cleanup, callback delivery |
| `008d5030` | `BSP_Settings_RefreshDownloadedContent` | ECX=settings, no stack arguments, `RET` | Optional manager creation, content operations, locale refresh |
| `007fae70` | `BSP_PlayerProfile_CommitContentState` | ECX=profile, no stack arguments, `RET` | DLC script/table, content names and mask, scene selection |

## Settings restore

`008d7a50` resets the manager at `0109cecc` through `00bd3450`, then requires
its byte `+21h`, a game object, and the profile's nonempty save-name predicate.
It copies game `+684h` (profile `+34h`), compares against the empty native string,
and calls storage virtual `+1Ch` with the name and `false`.
`00449af0` compares native length headers before considering `_stricmp`, so a
host string's nonemptiness is sufficient for this comparison against empty.

Only acceptance stores the callback in `00f88958`. The body reloads the game
pointer after the virtual query: a present game takes `00bd3d70` followed by
`006adb50(008d79a0)`; a now-absent game takes `00bd4380` and invokes `008d79a0`
directly. Immediate rejection calls the argument callback and leaves the global
slot unchanged. Host references captured by a deferred continuation must live
until delivery, just as the native implementation depends on global lifetimes.

`008d79a0` deserializes only when manager `+08h` is exactly zero. It takes globals
from the embedded Lua owner at `+38h`, constructs reader `004425c0`, calls
`008d6dc0` on settings `00f88980`, frees and clears buffer `+30h` when present,
closes the owner with `00b65e80`, and destroys the reader with `00441a20`.
After that branch, it clears `00f88958` before invoking its saved callback.
A callback that stores a new pending callback is therefore preserved.

The saved analysis originally had a false `CALL_RETURN` override on `_free`
at `008d7a02`. Bytes `008d7a07..008d7a10` are
`83 c4 04 c7 46 30 00 00 00 00`: `add esp,4; mov [esi+30h],0`.
The primary integrator repaired this under the Ghidra write lock and packet
owner, saved the project, and refreshed the export. The repair record is
`reports/profile_settings_restore_flow_repair.json`; no gaps remain.

## Content state

`008d5030` returns if both the download manager `00f8a304` and Xenon manager
`00f8abe8` are absent. With a selected user it attempts an `844h` allocation
and constructor `009955f0`, then rechecks the published manager pointer rather
than assuming construction succeeded. A manager with byte `+1Ch` set executes
virtual `+24h`, `0043ea80(settings+98h)`, virtual `+14h(settings+98h)`, and virtual
`+20h` in that order. Any present manager then registers locale table `globals`
and calls `00aa06d0(true)`, including a manager whose ready byte is clear.
The platform manager's owned records and these virtual implementations remain
required host operations. This routine does not apply all game settings.

`007fae70` starts with `008d5030`, constructs a Lua owner, opens it with mask
`4`, and runs `SCRIPTS/datatables/DLC.lua` through the existing script-with-
overrides operation with flags `0`. Mask `4` is the table-library request;
the state owner's mandatory bootstrap/base behavior belongs to its separate
implementation. It obtains `DLCTable` and releases the globals temporary.

The native profile list at `+CCh` is cleared and its mask at `+D8h/+DCh` is
zeroed before iteration. Each table **value** provides exactly 64 bytes:
the loop at `007faff0..007fb05d` starts at shift 63 and handles four characters
per pass until shift 0. Only ASCII `1` contributes a bit; all others contribute
zero. Each entry mask is ORed into the accumulated 64-bit mask. The **key** is
converted to a C string and appended to the profile list in Lua iteration order.
No sorting, deduplication, or connection to profile bonus `+ACh` is established.
`ProfileCommitState` supplies this list as a sidecar; the existing
`ProfileResetState::content_mask_d8` stores the mask.

The normal tail selects scene record `0` via `004c6890`, releases value, key,
and table objects, then destroys the Lua owner via `00b669a0`. Host handles are
released on each iteration, following the existing `GuiLuaReader` contract
that the Lua host holds its own cursor. RAII releases remaining references
before the owner during host exceptions; cleanup methods must not throw.

The host rejects null conversions or a value shorter than 64 characters with
`std::invalid_argument`. This is an explicit host safety boundary: native code
reads all 64 bytes unconditionally, even past a NUL. No behavior for malformed
native table contents or allocator exhaustion is claimed. Valid scripts must
provide `DLCTable` and suitable string data. Lua library implementation, VFS
overrides and the storage/platform backend are not duplicated in this packet.

## Evidence and verification

Each CLI analysis/export batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 little-endian 32-bit language and image base
`00400000`. Pseudocode plus assembly were inspected for all four bodies; the
assembly resolves register inputs, stack arguments, the 64-bit shifts, cleanup
and the incorrect `_free` return annotation. Saved exports remain ignored.

`./scripts/build.ps1` passes with MSVC Win32 Release. After `verify-seeds`, both
existing tests pass (`reconstructed_math` and `native_math_differential`). Those
math tests are general build checks, not differential tests of these routines.

One ignored local fixture, `local/profile_commit_fixture/main.cpp`, passes with
the real `GuiLua51Host`: deferred delivery, rejection leaving an old callback,
clear-before-call reentrancy, game disappearance after query, cleanup order,
failed manager publication, and exact content/locale/Lua/scene call order.
A synthetic two-entry table proves bit 63/31/0 accumulation and balanced Lua
handle release before owner close. The installed game's `DLC.lua` currently
contains an empty table; executing that file clears stale content names/mask.
The fixture checks calls into fake platform/storage hosts and does not execute
the original native functions. No game runtime or ABI compatibility is claimed.
