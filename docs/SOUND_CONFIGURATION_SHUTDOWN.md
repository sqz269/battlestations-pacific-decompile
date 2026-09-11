# Configured sound effects shutdown and storage destruction

This packet reconstructs the FMOD release policy reached from `00A882C0` via
`00A7F560`, plus the group/type storage destructors and the **shrink-only**
branches used by sound owner destructor `00A816B0`. The C++ functions operate
on the existing `SoundConfigurationState`; the extra native handle words use
the owner's live `words_144` array. Names are hypotheses, not recovered symbols.

## Evidence, ABI and coverage

The saved project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was verified by every live `bsp.py ghidra` query.
Pseudocode and assembly were inspected through `bsp.py show`; return tails
omitted from Ghidra were decoded from disk through `bsp.py disasm-raw`.

| Address and inclusive end | Native ABI | Coverage |
| --- | --- | --- |
| `00A7F560..00A7F63D` | ECX=manager, no stack arguments, RET at `00A7F63D` length 1 | complete release policy |
| `00A7AAF0..00A7AB84` | ECX=18h group record, no stack arguments, RET at `00A7AB84` length 1 | complete release policy |
| `00A7BFE0..00A7C051` | ECX=18h group record, no stack arguments, RET at `00A7C051` length 1 | complete ordinary storage-destruction policy; native allocator/SEH represented by C++ ownership |
| `00A7C6C0..00A7C731` | ECX=14h type record, no stack arguments, RET at `00A7C731` length 1 | complete ordinary storage-destruction policy; native allocator/SEH represented by C++ ownership |
| `00A7A980..00A7A9CF` | ECX=0Ch pointer-array header, signed stack count, RET4 at `00A7A9CD` length 3 | partial: shrink/equal path `00A7A9B6..00A7A9CF`; reserve/growth `00A7A98D..00A7A9B5` excluded |
| `00A7F240..00A7F2B7` | ECX=0Ch group-array header, signed stack count, RET4 at `00A7F2B5` length 3 | partial: shrink/equal path `00A7F28F..00A7F2B7`; reserve/growth `00A7F24E..00A7F28E` excluded |
| `00A7FCC0..00A7FD30` | ECX=0Ch type-array header, signed stack count, RET4 at `00A7FD2E` length 3 | partial: shrink/equal path `00A7FD04..00A7FD30`; reserve/growth `00A7FCCE..00A7FD03` excluded |

The three shrink interfaces require a valid header and
`requested_count <= current size <= INT32_MAX`. Their excluded reserve callees
are `00A7A8B0`, `00A7F160`, and `00A7FBD0`; they are not replaced by stubs.

Ghidra's stored `00A7BFE0` body ends at `00A7C019`, following the call to
returning CRT `_free` at `00A7C015`. Its omitted tail starts at `00A7C01A` and
ends with `RET` at `00A7C051`. Likewise `00A7C6C0` ends in the database at
`00A7C6F9`, with a disk tail `00A7C6FA..00A7C731`. Live function queries at
the four tail call sites returned no containing Ghidra function. These calls
are recorded separately as `outside_stored_body_calls` in the report, rather
than attributed to a body that Ghidra did not contain. No worker-side Ghidra
mutation was performed.

## Release policy and live rereads

`00A7F560` selects the last group using native manager count `+12C`, releases
that group's effects, then **rereads** the outer count and selects the current
last group for storage destruction. It destroys storage before decrementing
the outer count. This continues until zero. It preserves the outer backing
array and capacity `+130` for the owning destructor to free later.

For each group, `00A7AAF0` removes the current last DSP, optionally gets memory
statistics, rereads the current count/backing/handle, and releases that current
last DSP. It optionally gets statistics again, rereads count, and decrements
only when nonzero. It finally releases `group+08` even when that handle is null.
It does not clear the group handle or free the DSP array backing.

Every result in `00A7AAF0` is checked only against `2Bh`. A match invokes the
two-output `FMOD_Memory_GetStats` import; other errors do not stop teardown.
Callbacks must preserve the validity of the active group object and any DSP
element the subsequent native code reads. The projection deliberately makes
no additional null/empty guard before the native back-element reads.

After the group loop, the owner-handle order is:

1. `+14C` (word 2): if initially nonzero, remove, reread, release; leave the word.
2. `+144` (word 0): if nonzero, release group, then write zero.
3. `+148` (word 1): if nonzero, release group, then write zero.
4. `+150` (word 3): if initially nonzero, remove, reread, release; leave the word.
5. `+154` (word 4): if initially nonzero, remove, reread, release; leave the word.

These extra-handle calls ignore **all** results and perform no memory-stat
diagnostics. The master channel group at `+140` and the system DSP list at
`+134` are not visited by `00A7F560`. Their ownership must not be inferred from
this function. This packet adds no FMOD release to the system-list shrink path.

## External contracts and call-site evidence

Import thunks were inspected independently: `00C2DE30`, `00C2DE2A`,
`00C2DE24`, `00C2DDEE` jump through IAT slots `00CE2530`, `00CE2534`,
`00CE2538`, `00CE255C`, respectively. Live prototypes identify the first
three as one-object-pointer stdcall methods, and statistics as stdcall with
two output pointers. The implementation does not recreate the DLL internals.

| Host operation / native callee | Sites | Containing stored function |
| --- | --- | --- |
| `dsp_remove`, `00C2DE30` | `00A7AB0F` | `00A7AAF0` |
| `dsp_release`, `00C2DE2A` | `00A7AB37` | `00A7AAF0` |
| `channel_group_release`, `00C2DE24` | `00A7AB67` | `00A7AAF0` |
| `memory_get_stats`, `00C2DDEE` | `00A7AB23`, `00A7AB4B`, `00A7AB7C` | `00A7AAF0` |
| `dsp_remove`, `00C2DE30` | `00A7F5BF`, `00A7F60F`, `00A7F62B` | `00A7F560` |
| `dsp_release`, `00C2DE2A` | `00A7F5CB`, `00A7F61B`, `00A7F637` | `00A7F560` |
| `channel_group_release`, `00C2DE24` | `00A7F5DB`, `00A7F5F5` | `00A7F560` |

The report additionally records each internal reconstruction/storage call with
its containing function. All stored-body rows are checked against live bodies
and exact instructions by `tools/verify_report_calls.py`.

## Storage representation and uncertainty

The record layouts reuse `sound_configuration.hpp` and the initialization
evidence in `SOUND_CONFIGURATION.md`. Group producer `00A7DE10` copies the
name at record `+00/+04` and passes record `+08` as the FMOD output handle at
`00A7DE77`; a group is 18h bytes. The type record is 14h bytes with name at
`+00/+04` and listener-route pointer array at `+08/+0C/+10`.

Both storage destructors resize their inner pointer arrays to zero and call
CRT `_free` on the backing. `00A7A930` and existing reconstructed `00A7B3D0`
were inspected: their zero branches reduce count without deleting pointees.
The omitted tails inspect the name pointer at `+04`, call pool singleton
`00419CC0`, then return that block through `00BD1510` with
`(pointer, native length+1, 1)`. The singleton takes no arguments and leaves
the already pushed release arguments on the stack; `00BD1510` consumes 0Ch
with `RET 0Ch` at `00BD152F` and `00BD156C`. The third argument is not read.
The pool helpers remain established native allocator contracts.

The projection releases the `std::vector<void*>` backing before the
`std::string` backing. It does not reproduce pooled allocation traffic, SEH,
or dangling native fields after destruction. The C++ owned containers become
empty; still-live scalar handle/capacity fields are not zeroed by storage
helpers. Shrink helpers transfer a removed record's C++ storage into a local,
decrement the visible container count, then release that storage. This preserves
native count-before-destruction policy without introducing a second owner.
Outer backing storage and logical capacities survive shrink operations.

## Validation boundary

The standalone Win32 `local/effects_shutdown_probe.exe` is built with MSVC
and `/MANIFEST:EMBED`. One focused fixture passed: reverse release order,
handle rereads, count reread after release, diagnostics only for `2Bh` in
group release, ignored extra-handle errors, retained master/system state,
and storage-only shrink retaining the surviving prefix and capacities.
It is an uncommitted integration aid, not a native differential test.

The complete `scripts/build.ps1` Release Win32 build passed, including the
existing `reconstructed_math` CTest (1/1). This packet has no installed-game or audible-output validation,
and none of these projection interfaces is a drop-in native ABI replacement.
