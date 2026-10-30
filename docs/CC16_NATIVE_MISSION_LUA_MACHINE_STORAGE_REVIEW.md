# Independent raw mission Lua machine storage review

Reviewed immutable candidate `041540774d60ad7b95c59496795b97fb16d807ae`, based
on `9e360e71494881f48b7944862b7777f69ef5ac9d`. Review baseline is Main
`3fe6fe7eb9ae9528341f3291897cbb26988c0182`; native reference/readiness is immutable
`414237d0820620782aeb28760b63fff825e74b4f`. No mutable candidate was inspected
before root supplied its immutable hash.

**Pass for the bounded explicit Source constructor/scalar contract. No blocking
source or emitted-code finding.** Independent strict MSVC Win32 object compile,
complete physical COFF inspection and actual provider checks support root's
common-build/admission review. This does not approve native ABI/EH/fault
equivalence, safe recovery of a foreign C++ escape, raw host/table adoption or
gameplay. Candidate source/contracts are unchanged.

## Constructor: complete normal and partial prefixes

The full frozen native constructor is 118 bytes/37 instructions. The Source API
operates on genuine caller-owned 0Ch backing, without a projected `LuaMachine`
cast or another machine allocation. Its only receiver writes are DWORD profile
+0, the returned state pointer+4, and final owns BYTE+8. Bytes+9..B and the
earlier owns preimage are not initialized or widened into a DWORD store.

| Epoch | Independent result |
| --- | --- |
| Profile/new state | Store numeric CF8394, call real `luaL_newstate`, publish its result at+4. CF8394 remains identity data, not a callable Source table or heap credential. |
| Panic argument | Pass the captured newly returned state, rather than reloading+4, to real `lua_atpanic` with the existing `cc13_seg29::lua_machine_panic_006b8720`. |
| GC | Reload CURRENT state+4 after panic installation, then `lua_gc(state,6,100)`. |
| Each library row | Reload CURRENT state+4 separately for push-closure, push-string and unprotected call. Preserve opener/name/next-opener read order through volatile Source rows. |
| Actual table | Seven real linked functions in order: base with empty name, table, io, os, string, math, debug; zero upvalues, call arguments1/results0. No package, `luaL_openlibs`, guarded-opener substitution or numeric original code address. |
| Success ownership | Only after all seven calls and the null terminator does emitted code store BYTE+8=1 and return the captured base. |

There is no null-result recovery, catch, automatic close, rollback or success
substitution. If state creation or a later API fails, the raw prefix stays as
reached. An inherited owns preimage is not a valid constructor-success marker.
The caller must retain the actual allocation, current state and independent
provider/callback/operation custody. Native current-cell reloads also permit
the current state to differ from the initially created state; the primitive
must not silently repair, close or lose custody of an orphaned earlier state.

The reused panic callback is a genuine compiled Source C-ABI-compatible
callback for linked Lua: real gettop, tolstring(top,null), return 0, with no pop,
logging or duplicate panic implementation. Its Native ECX entry transport is
explicitly not supplied by the new ordinary C++ interface.

## Scalar: capture, close, flags and free

The frozen native scalar is 45 bytes/15 instructions. Independent emission proves:

1. Capture owns BYTE+8 before restoring profile CF8394. All nonzero owns bytes
   select the owned branch; owns0 does not read state+4.
2. For captured nonzero owns, reload CURRENT state+4 and call real `lua_close`
   with no null guard. Do not reload owns after that callback.
3. Only after close returns, read the low byte of the Source by-value flags
   word through its volatile byte lvalue. Emitted offset1E performs that read
   after the close CALL at offset0x16 and its stack adjustment.
4. Bit0 selects `singleton_lifetime_free(captured_base)`. Return the original
   address bits; do not dereference the receiver after free.

The Source does not clear state, owns or padding, nor restore fields changed by
close/finalizers. Flags0 still closes an owned VM and retains stale machine
fields; replay is not admitted. A failing or nonlocal close does not reach the
flags read/free. Flags1 requires the actual matching canonical outer allocation;
there is no inferred foreign-heap free. The volatile byte read pins this Source
epoch without claiming Native caller-stack placement or private-stack aliases.

The outer machine allocation and Lua VM allocator are separate obligations.
The genuine host allocation is canonical `singleton_lifetime_allocate` object12;
its scalar free is `singleton_lifetime_free -> std::free`. The VM is retired by
its own linked `lua_close`/allocator, never by freeing the state pointer as the
outer object. No catch-time outer free or implicit failed-constructor cleanup
has been added.

## Actual Lua 5.1.1 and failure boundary

The fetched actual header declares Lua5.1.1 and GC pause operation6. The CMake
recipe pins the upstream archive hash, compiles the28 C units, and applies only
the established long-string compatibility setting. The relevant 15 fetched
header/Source pins still match the candidate/readiness evidence.

The actual `luaL_newstate` provider uses `lua_newstate` with its real realloc/free
allocator and can return null. The candidate deliberately does not fabricate a
valid state or add a null-success branch. Actual Lua's C build uses setjmp/
longjmp; the physical `luaD_throw` member imports real longjmp, setjmp and exit.
With no protected error frame, the real library invokes panic and then exits
even if that panic returns0. The existing panic callback is not recovery.

Actual `lua_close` invokes finalizers, internally handles Lua GC-finalizer errors
under that library's protection, and frees the state after normal completion.
This is the actual library policy, not a newly reconstructed vendor body.
It does not establish safe restoration after a foreign C++ exception through
Lua C frames. Retained backing is diagnostic custody; automatic close/replay
requires independently valid interpreter state and live finalizer/services.
This qualification is present in the candidate contract and is retained here.

## Strict compilation and whole emitted evidence

The reviewer extracted the two Source blobs from Git into ignored local frozen
inputs. All five author artifacts match the immutable report before use.
One independent `/c` compile passed using `/std:c++17 /O2 /Ob2 /DNDEBUG /MD
/EHsc /W4 /WX /fp:strict`, plus actual include dependency capture. No candidate
body, Lua state, test, executable or DLL was executed.

The independently parsed i386 COFF defines:

| Root | Complete emitted body | CALL relocations |
| --- | ---: | --- |
| constructor |123 bytes| real newstate, atpanic, GC, pushcclosure, pushstring, call |
| scalar |52 bytes| real close, canonical singleton free |

Both complete code byte strings match the author's whole emitted bodies.
Independent symbol-index resolution confirms the 64-byte table's 14 DIR32
relocations to seven genuine names/openers, followed by eight zero terminator
bytes. The constructor has five distinct+4 accesses (one publication, four
current reload sites), a final byte owns store, and no padding stores. The
scalar's post-close byte flags read and captured-base free are actual emitted
instructions. No original numeric code target or callable table is installed.

Actual compiler dependency capture pins one frozen Source file and 86 included
headers. Eight declared in-house dependency pins match. The actual Lua archive
still matches its recorded hash; all 16 required APIs/openers have independently
verified physical defining members. `luaD_throw` is additionally verified.

The primary Core archive advanced during concurrent root integration, so its
whole hash no longer matches the author's older snapshot. This is recorded,
not treated as an unchanged-input build. Its actual panic, allocate and free
symbols are independently confirmed in physical defining members. These are
provider-availability checks, not fresh Source-to-entire-archive attestation.
Root's common build, registration, actual entry linking and admission remain
separate from this review.

## Scope and disposition

Accept this immutable candidate for root's bounded Source/common-build review.
The candidate commit itself adds no CMake registration or caller selection.
Only the two review evidence files are committed here; no candidate, shared
metadata, address ownership, guard, Ghidra program, installation or peer changes.

Still open: genuine 00884BE0 host adoption and callback custody, authenticated
CF8394 scalar transport, raw D0E7A8 host/queue/payload/vector lifetime, application
Game ownership, full Native fault/FH3/SEH/ABI, startup and gameplay gates. A numeric
profile does not make the existing generic current-table dispatcher callable.
New unique native-body, duplicate-body, vendor-body, ABI and activation credit
are zero. Names remain descriptive hypotheses.
