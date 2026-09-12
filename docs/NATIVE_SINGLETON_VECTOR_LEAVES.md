# Native singleton vector leaves

This packet reconstructs all six raw vector leaves: **BCF910[19], BD0160[30], BD0180[47], BD0220[42], BD0500[48], BD0560[46], totaling 232 bytes**. They operate on native DWORD storage and preserve the general count, aliasing and malformed-distance paths. They are dependencies for the complete raw singleton manager; the manager, its registry getter and registered-owner dispatch remain separate work.

The implementation is in [native_singleton_vector_leaves.cpp](../src/native_singleton_vector_leaves.cpp), with the public contracts in [native_singleton_vector_leaves.hpp](../include/bsp/native_singleton_vector_leaves.hpp). Descriptive names are reconstruction hypotheses, not recovered symbols. No typed manager cast, private manager allocation, validation callback or replacement vector is introduced.

## Native evidence and ABI

The existing project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language and image base were verified by the guarded `tools/bsp.py ghidra` commands before their analysis batches. Every byte of all six complete bodies was captured anew and independently decoded from the installed PE; all 232 bytes match saved Ghidra memory. The original PE SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The prior discovery seal `d992c6b6d68ae04a03e4be9a8b3d09657e8e5a0dd3482d56f9171ef127ad2d8d` and all its 276 artifact/source pins were rehashed before reuse. The new capture additionally covers complete BF67A7[99], BF65AC[5], BF9DC8[142] and BF66EF[36], giving ten spans / 514 bytes. The audit records the new evidence and build-artifact seals.

| Native entry and inclusive extent | Input, result and return instruction | Full behavior |
| --- | --- | --- |
| BCF910..BCF922 / 19 | ECX owner; EDX unused; EAX count; RET | Capture begin from owner+4. Null gives zero without reading end. Otherwise load end+8, subtract begin modulo32 and arithmetic-shift right2. Return the signed result's raw DWORD pattern. |
| BD0160..BD017D / 30 | cdecl `(first,last,value_slot)` at entry ESP+4/+8/+C; RET | Capture the endpoints. Until address equality, reload the DWORD at the captured value-slot address and store it at the current destination, advancing by4 modulo32. Preserve ESI. There is no signed/unsigned order check. EAX happens to finish at last; the source exposes no return value. |
| BD0180..BD01AE / 47 | cdecl `(first,last,destination_end)`; EAX destination-begin; RET | Compute `(last-first) SAR2`. Derive destination-begin modulo32 for every result. Only a signed-positive count calls memmove_s with `(derived_begin, count*4, first, count*4)`. Ignore service status and return captured derived begin. Preserve ESI. |
| BD0220..BD0249 / 42 | ECX owner; EDX unused; RET | Retain owner in ESI, capture current begin+4 and free it only when nonnull. After free returns, zero +4, +8, +C in order. Preserve +0 and +10. |
| BD0500..BD052F / 48 | stdcall `(first,last,destination)`; EAX advanced destination; RET0C | Compute raw32 `(last-first) SAR2`, derive count*4 and advanced destination modulo32. Only zero skips memmove_s; negative distances still call it. Ignore service status and return the pre-call advanced destination retained in ESI. |
| BD0560..BD058D / 46 | stdcall `(destination,uint32 count,value_slot)`; EAX advanced destination; RET0C | For full unsigned count, reread the DWORD value for every four-byte store. Count zero skips the value-slot load. Return initial destination+count*4 modulo32. Preserve ESI/EDI and, on the loop path, EBX. |

Incoming ECX is not consumed by BD0500 or BD0560. The source fastcall declarations for the two ECX entries include an explicitly unused EDX argument; both still emit plain RET with no stack arguments. Copies pass four cdecl words to memmove_s and clean exactly10h bytes. Cleanup passes one cdecl word to the existing free service and cleans4 bytes.

BD0500's JE at BD051C consumes flags from the **SAR** at BD050E. The intervening LEAs and PUSH do not change those flags. BD0180 instead executes TEST and JLE after deriving destination-begin. BD0560 uses TEST/JBE and TEST/JA, so all nonzero DWORD counts enter its loop. The source keeps these sequences rather than replacing them with a signed loop or pointer subtraction.

The old saved Ghidra listing still omits `BD0230: 83 C4 04 / ADD ESP,4`, although the full saved byte span and installed PE both contain it. The new complete decode and source retain the instruction and all subsequent zero stores. This worker performs no Ghidra mutation; annotation and listing repair belong to the integrator.

## Concrete memory and free boundaries

Both copying entries call the actual SDK `memmove_s` from `<cstring>`. Its body is the existing static-inline definition in `corecrt_memcpy_s.h`, not a packet-defined substitute. The native BF67A7 body and this service both accept zero byte count first, reject null destination/source with EINVAL, reject insufficient destination size with ERANGE, and use actual overlapping `memmove` on the valid path. Neither failure path clears the destination. The leaves pass equal destination-size/count words and discard the result exactly as native does. They do not validate or silently exclude negative distances, null copy pointers or other malformed storage.

The error ownership is explicitly a **source CRT service boundary**. Original BF67A7 writes errno through BFFB8B and passes five zeros to BF66EF, which decodes current global109DD64 and invokes its current handler or follows its Watson path. The source SDK body writes the owning source CRT's errno and invokes `_invalid_parameter_noinfo`; UCRT selects its current thread-local handler, then its current global handler, then its Watson path. Returning handlers continue through the actual service and the leaf returns its precomputed result. This packet neither binds original errno/encoded-handler state nor introduces private invalid-handler state. Arbitrary original CRT exception/SEH behavior, native service-internal flags and fault sites are not claimed.

Cleanup calls the existing `bsp::singleton_lifetime_free`, whose reviewed implementation directly calls `std::free`. Its pairing is the existing `singleton_lifetime_allocate` implementation's real malloc/new-handler-retry service. The raw storage must belong to this source CRT allocation domain; a pointer into the game's original private CRT heap is not made compatible by the shared DWORD representation. Original BF65AC is a returning jump to complete BF9DC8, which contains both small-block-heap and HeapFree paths. A source std::free call does not reconstruct those original internals. No allocation or ownership domain is added here.

## Verification and limits

The source uses MSVC Win32 naked bodies, preserving original register use, caller/callee stack cleanup, raw32 arithmetic and actual DWORD accesses. This avoids C++ subtraction of unrelated pointers and typed alias assumptions, including aliases between destination and value-slot storage. No guard, shortened count path or handler repair is inserted. Function addresses, literal machine fault addresses, original native EH/SEH frames and service-internal machine state remain outside the source ABI claim.

The strict `scripts/build.ps1` Release Win32 build passed with MSVC 14.51.36231, SDK 10.0.26100.0, `/MD /W4 /WX /fp:strict`, and both existing CTests passed. All eight seed ranges match the installed PE. The six COFF bodies match **all 232 native bytes exactly except the three named call relocations**: BD01A3 and BD0522 to actual SDK `_memmove_s`, and BD022B to existing `singleton_lifetime_free`. This includes every complete return instruction and BD0230's stack adjustment. Exact copies of the selected two objects occur as members of the actual `bsp_core.lib`.

Each leaf occupies one isolated code section of exactly its native length, with no trailing code. All seven nonempty code sections in the owned object are classified: the six leaves plus the complete 94-byte SDK memmove_s body, totaling 326 bytes. The proof asserts all 33 helper instructions, branches, argument pushes and exact relocations to `__imp___errno`, `__imp___invalid_parameter_noinfo` and `_memmove`. It also asserts the entire existing 6-byte free-provider body and its sole jump relocation through `__imp__free`. Full helper/free binaries and reviewed listings are pinned.

The actual build's import libraries map memmove to `VCRUNTIME140.dll`, errno/invalid handling to `api-ms-win-crt-runtime-l1-1-0.dll`, and free to `api-ms-win-crt-heap-l1-1-0.dll`. The same imports are present in the existing built `bsp_game.exe`. These are provider-binding checks: the new leaves are unreferenced members of `bsp_core.lib`, and no claim is made that the game executable links or executes them.

The [audit](../reports/native_singleton_vector_leaves_audit.json) records full native/COFF mappings, captured compiler commands and input dependencies, provider bindings, exact archive membership, and the immutable evidence seal. The worker's ignored CMake hook adds only its owned source; the integrator owns shared CMake registration, ledgers, names and Ghidra annotations. No new tests were added and no game or ad hoc executable was run. Existing math CTests and seed comparisons provide regression evidence, not execution proof for these leaves. Complete raw singleton manager construction/insertion/destruction and current mixed-owner dispatch remain open.


Primary integrated the complete source into main and replayed the reviewed
static verifier against actual archived objects. All232 native leaf bytes except3 explicit calls match exactly; complete94-byte/33-instruction SDK memmove_s and6-byte free provider checked. All7 owned CODE sections326bytes, four actual frozen CRT import libraries and two exact archive members verified. Main19-input snapshot was post-build; four source/header files exactly match worker prebuild pins and185 actual compiler inputs are frozen. The final combined
main library also contains the two six-leaf proof objects byte-for-byte; both
main builds passed the two existing CTests and eight reference seeds. Ghidra
call-flow overrides at BD022B were cleared, with prior values recorded; complete
post-free instructions, saved names/comments and refreshed exports are present.
Primary evidence is frozen under `local/raw_vector_primary/`. No lifecycle
fixture, original static-CRT/SEH compatibility or game behavior is established.
