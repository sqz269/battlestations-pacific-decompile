# Known-found opaque enum dictionary words

Packet `cc11_scene_enum_dictionary_word_getters` binds the complete normal
success contracts of two CString getters to the existing raw string and borrowed
dictionary providers. The four new files are disjoint from the prior lookup
integration. Names are hypotheses; no native library owner or enum identity is
constructed by this packet.

| Entry | Inclusive end | Exclusive end | Bytes | Source function |
|---|---|---|---:|---|
| 0048E960 | 0048E9E0 | 0048E9E1 | 129 | `read_native_enum_table_word_0048e960` |
| 0048E840 | 0048E8C0 | 0048E8C1 | 129 | `read_native_enum_symbol_word_0048e840` |

The whole assembly, including prolog, normal cleanup and return, was inspected
read-only in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Both complete
body bytes equal the installed executable. SHA256:

- 0048E960: `9ffe199f6c9124ae8bc8a92939ba39ad6f943ca21dc5483d5f464a0593590027`
- 0048E840: `1af840ac4f1fcd6434fc3b8f923ab125e771ab4efb7c7d0af2350de8106cb62f`

## Complete native normal contract and Source binding

Observed input roles are ECX owner and one stack CString; terminal RET4 is at
`0048E9DE`/`0048E8BE`. Both bodies establish an FS:[0] exception frame, reserve an
eight-byte temporary header, preserve ESI, and call `0041E870` with the CString.
They pass that temporary to `0048D4E0`/`0048D480` with receiver **owner+4**. The
lookup's out-bucket reuses the native incoming CString argument stack slot after
the CString has already been copied; this is not a caller-string mutation.

At `0048E9A3`/`0048E883`, the body unconditionally captures the DWORD at returned
node+8. A missing node causes a null+8 faulting read; there is no default,
insertion, type conversion or optional-value branch. Native exception handling
and the ultimate fault outcome remain unproved. An actual found word of zero is
valid and distinct from a missing node.

Normal cleanup loads the temporary data and, when nonnull, captures length+1,
passes `(data, size, 1)` through `00419CC0` then `00BD1510`. Null data skips both
calls. The body marks the cleanup state inactive, restores its prior FS:[0],
returns the captured word and consumes one DWORD. The word is captured **before**
cleanup. These boundaries are assembly receipts, not formal recovered ABI types.

Source explicitly constructs one real temporary with
`construct_native_string_header_0041e870`, uses the existing complete finder on
owner+4, reads node+8 as `uint32_t`, and explicitly destroys the temporary with
`destroy_native_string_header_0041dd20`. That existing raw-context destructor
performs the same nonnull-data, captured-size, current-pool normal return path.
`NativeString` has no implicit buffer cleanup, so this introduces no invented
automatic native unwind/rollback policy. The new C++ interface additionally
requires real raw-pool context and the two distinct empty-cell bindings.

The lookup/data/comparison dependencies are the accepted complete normal
`0048D4E0`/`0048D480`, `00489610`/`004895B0`, `00419CA0` and `00438E10` bindings.
Current Source `toupper`/`_stricmp` are admitted only for closed ASCII content
and the C locale. The original FS:[0] scaffolding, handler addresses
`00C62BD8`/`00C62B98`, failure cleanup and original constructor/getter/pool-return
machine-code graph are **not** executed or claimed by this Source packet.

## Actual caller and value roles

The traffic fragment inside `0049CF80` passes `[00E1867C]` and
`LandVehicleClasses` to `0048E960` at `0049D25A`, then forwards its returned word
as ECX to membership `0048E8D0` at `0049D261`. The vehicle hit path repeats the
table read at `0049D27D`, then forwards the pointer to symbol getter `0048E840`
at `0049D284`. The miss path reads `SoldierTypes` at `0049D2AC` and the symbol at
`0049D2B3`. Thus the table word is a borrowed **table pointer** in this context;
the ordinal comes from the separate symbol getter. Vehicle/soldier materializers
`00964790`/`004B1400` remain separate dependencies.

`008F67B0` guards a table getter at `008F699E` with prior table membership at
`008F697C` and reopens that actual table for declaration parsing. Typed enum
parser `008F5A00` uses the getter at `008F5FA2` on its existing-table path and
forwards the table pointer into declaration construction. None of these receipts
establish Source enum record identity or authorize a global table default.

Current Source `PropertyLibrary` is private vector/map storage with semantic
`resolve_symbol`. It has no native raw dictionary owner and is not converted to
these borrowed getters. `SceneTrafficHost` still lacks a concrete production
implementation; the traffic loader/class/hierarchy/runtime connection remains
unbound. The old traffic comment that an absent symbol getter returns whatever
the table holds is inaccurate: the native node+8 read has no miss guard. This
packet records that correction without editing the unowned traffic source.

## Focused Source/compiler/lifetime proof

The unique ignored fixture is
`local/cc11_scene_enum_dictionary_word_getters/word_probe.cpp`, with
`run_probe.py --out <fresh-directory>`. Prior lookup `run05` and its support
artifacts remain immutable. This packet independently froze current primary b02
libraries with equal original-before/copy/original-after hashes, and verified
the pinned copies before/after link/run. No live-build libraries were linked.

`run02` strict-compiled seven fresh Win32 TUs: getter, lookup, native_string,
string_pool_storage, string_pool_owner, scene_file and fixture. `/W4 /WX
/fp:strict` compilation, link and run all exited zero. There are 38 actual
production header includes plus six production CPPs: 44 unique production
inputs, or 46 including fixture CPP and recipe. The extracted embedded manifest
is `asInvoker`; the report records hashes and map attribution. No tracked tests,
shared metadata, Ghidra writes or full worker build were used.

The actual Source lexer extracts installed `global.enums` LandVehicleClasses
(22 symbols) and SoldierTypes (6). Real raw-pool constructors create all query
and node key buffers. Externally backed 14h slots use only the insertion-proved
key+0/+4, opaque mapped+8 and next+0C fields; owners supply only the observed
borrowed field extent. This is not original map construction/insertion/lifecycle
or a native CEnum sizeof assertion. The table getter's returned opaque pointer
is forwarded directly to the real symbol getter, yielding installed word bits,
including `Us_jeep=466`, `Us_ambulance=465`, `None=FFFFFFFFh` and soldier zero.

All **62 known-found getter calls** pass: 60 installed authored/case-variant
table/symbol reads and two found empty-key reads. Each call primes a real
exact-size cached buffer using the genuine constructor/destructor. For identical
text the existing raw-pool pop+return rule must restore its full prefix before
the OS critical-section fields; byte equality is checked after every call.
Empty keys perform no allocation/release and leave that prefix unchanged.
Owner bytes, complete fixture nodes and owning key buffers remain unchanged.
An underlying missing lookup returns null, but the known-found wrapper is never
invoked on that miss and no native fault/EH parity is inferred.

The distinct `00E186ED`/`00E17654` cells are checked against saved Ghidra and PE
**image-initial zero-fill**, mapped only into a free range, and protected
read-only. This is Source fixture input, not a captured live-game global. The
first recipe attempt detected file-backed-versus-zero-fill handling before
compilation; `run02` explicitly records the image zero-fill distinction.

Admission requires found keys, valid stable actual Win32 owner/node/header
storage, closed ASCII CStrings, C-locale providers, distinct valid empty cells,
the real returning raw pool and nonaliasing/no-reentry/no-concurrency conditions.
Missing/invalid/null keys, allocation failure, namespace/type conflict, native
enum identity/ownership, EH/SEH/fault behavior, whole ABI and gameplay remain
outside the claim. Full main registration/build is pending primary integration.

Primary integration at `e240b8c81d6d43464ae5eb6a6e9db8b7c56cd956` passed the complete MSVC Win32 build and all three existing CTests. The independent current-library fixture freshly compiled 7 actual TUs and passed 62 known-found Source getter calls, with 45 Source/header/fixture pins and 38 actual compiler includes. All Source, recipe, installed inputs, three current support libraries and the original PE remained unchanged. New Source COFF, exact native byte agreement, manifest, logs and receipts are recorded in the report.
The current public Source getters each emit 96 bytes/29 instructions with direct constructor/finder/destroy calls; the unguarded node+8 word load at49h precedes destroy at51h. Original wrappers are not replayed; declaration identity and production traffic binding remain unbound.
