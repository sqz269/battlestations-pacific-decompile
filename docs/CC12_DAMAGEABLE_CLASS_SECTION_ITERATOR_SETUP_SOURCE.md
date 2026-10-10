# DamageableClass Sections iterator setup and pair cleanup

Addresses: 0087CDA9 0087CDAD 0087CDBE 0087CDDC 0087CDED
0087D1CF 0087D1DB 0087D1EC. Enclosing original: 0087CA80.

The bounded fragment has ordinary Source using the genuine Lua-object
constructor, first-iteration adapter, key-unbound predicate and destructor.
A fixed caller-retained owner keeps the actual iterator key/value alive across
the excluded loop. The worker focused compile, complete candidate/provider
COFF review and existing registered baseline checks passed. The candidate is
not registered or admitted by the integrator yet. No whole original function,
Sections loop or complete parent reader is added.

| Region | Exact bytes | Instructions | Excluded successor |
| --- | --- | --- | --- |
| Setup/first pair | 0087CDA9..0087CDFF, 87 bytes | 19 | CE00, start of row/loop body |
| Pair-only cleanup | 0087D1CF..0087D1F0, 34 bytes | 6 | D1F1, saved row-register reload |

Setup SHA-256 is
`748296e9fb448f5d13e1f279d0c64e71dbea16f1b550367b1267993b192ebeeb`;
the pair cleanup is
`f0b5db11cb942419328f6db88be831c949228f5190101828ef74d4508ed63d39`.
The six-byte LEA EBX,[EBX] alignment instruction at CDFA is included.
The row clear at CE0D, vector append at CE48, all row/effect/category work,
next iteration at D1B1, row-register reload D1F1 and outer cleanup D1F8 are
excluded. Returning false from this fragment does not close the outer fields.

The complete enclosing 3,238-byte body and all 858 instruction starts match
installed PE, saved listing and fresh live listing. Its SHA-256 remains
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
Live queries used the verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` client. The retained 37-state FH3 audit supplies
the lifetime maps. No new Native child, data, handler or FH3-data opening was
needed. The report pins the old audit and complete fresh parent gate.

The native parent takes actual descriptor in ECX and actual row on the stack,
RET4, with E4h locals and four saved registers. The fragment starts at an
interior instruction, not a callable native entrypoint. Let S denote ESP after
that allocation/save sequence. Actual key is S+58h, value S+2Ch, Sections
S+80h, Damage S+44h and Unique S+94h. Each object occupies 14h bytes. The
key/value slots are fresh with stale bytes allowed, stable, disjoint and
aligned at least four bytes. Existing outer objects and their owners remain
live throughout. No Lua header, owner, current index or tracked slot is copied.

| Native site | Operation and decimal state |
| --- | --- |
| CDAD | Construct actual key at S+58h through B65F50 |
| CDB6 / CDBE | Raise to12, then construct actual value at S+2Ch |
| CDD4 / CDDC | Raise to13, then first iteration on actual Sections, key and value |
| CDED / CDF4 | Test actual key unbound; empty jumps to D1CF |
| CE00 true continuation | Both actual pair objects remain live under13; body excluded |
| D1D3 / D1DB | Lower13->12, then destroy actual value with B67700 |
| D1E4 / D1EC | Lower12->11, then destroy actual key with B67700 |

Retained state12 has parent11/action C9697E and cleans EBP-A8h = key S+58h.
State13 has parent12/action C96989 and cleans EBP-D4h = value S+2Ch. The
enclosing successful Damage entry owner still owns Sections/Damage under11,
and the outermost Unique owner remains responsible for Unique under1.

`NativeDamageableClassSectionIteratorSetupFragment` cannot be copied or moved.
Its scratch borrows the successful Damage entry owner, actual Sections, and
actual fresh pair storage. The caller creates the pair owner before its one
allowed `open()`, nested inside the Damage entry owner's lifetime. Its own
constructor initializes state11. Actual key construction precedes state12,
actual value construction precedes state13, and state13 precedes first iteration.
The real object constructors perform no allocation and are noexcept on valid
storage; their transitional state12 is retained in compiled code. Hardware
fault/SEH behavior on invalid storage is outside this ordinary Source contract.

False from `open` means the first iterator is empty and both pair objects have
already been closed to state11. True means a current pair exists and state13
remains live. The caller retains this fixed owner through the entire excluded
loop and all inner guards, then explicitly calls `close()` at the matching
normal pair tail. Only afterward may the outer Damage entry owner close
Sections/Damage at its own tail. No successful-return cleanup or substitute
continuation callback is inserted. Borrowed-owner references do not perform
runtime admission checks: the successful outer state and lifetime are required
caller preconditions, and complete caller composition remains held.

An explicit value cleanup lowers to12 before calling the genuine destructor.
If it throws, ordinary owner unwinding closes the remaining key once after
lowering to11, without retrying value. If key cleanup throws, state11 prevents
retry. First-iteration failure leaves state13 and requires ordinary owner
unwinding before further work. The pair owner never cleans outer objects.
Reopen, replay, callback reentry and scratch-binding changes are forbidden.
Secondary failure in its noexcept destructor follows ordinary termination;
native FH3, SEH/longjmp and double-exception/fault identity remain held.

The actual Source providers are all in `src/native_lua_objects.cpp`:

| Provider | Preserved behavior |
| --- | --- |
| B65F50 constructor | Placement construction into actual 14h storage; owner null, kind0, index -1, tracked0; opaque0C/padding remain stale |
| Protected B67080 first iterator | Release value then key, protect same-frame checkstack/pushnil/next, publish actual key then value addresses |
| B66420 key-unbound predicate | Read actual key kind04; true exactly when zero |
| B67700 destructor | Actual tracked release/index updates, then current kind clear; kind0 skips owner access |

`docs/NATIVE_LUA_READER_QUERIES_CC10.md:43-44` explicitly records the B66420
stack-key argument and ignored ECX. Thus the parent's table in ECX at CDED
does not change which object is tested; the existing Source key reference is
the established adapter. No new native helper inspection or guessed ABI is
used. `docs/NATIVE_PARTICLE_LUA_PRIMITIVES_ORCH4_R25.md:29-33,49-89` records
the iterator and protected-error contracts, and is pinned with the Source.

The first adapter starts protection after value/key releases. Lua failure
removes only operation/error temporaries and retains prior stack removals and
index shifts; it does not restore removed objects. Here both constructed
objects start kind0. After successful lua_next, nonallocating getters and
tracking stores publish key then value; no linked Lua error occurs between
those publications under the valid storage contract. First iteration does
not invoke __pairs or an indexing metamethod. The caller keeps the actual
table, interpreter and stack/tracking domain stable and satisfies unchecked
capacity limits. An inherited error handler stays below every removed or
consumed slot, including surrounding cleanup; no handler relocation is added.
Lua5.1.1 and MSVC runtime remain genuine library dependencies.

The focused compile used `/O2 /Ob2 /Oy- /MD /W4 /WX /fp:strict /EHsc /std:c++17`
and Win32/Windows/Release defines. The actual provider TU was freshly compiled
with the same flags and real Lua5.1.1 headers. The report records physical
section indices, all symbols/auxiliaries, every relocation, raw section bytes,
complete candidate decoding, destructor EH/SafeSEH and compiler metadata.

| Candidate physical section | Body | Bytes |
| --- | --- | --- |
| 4 | Constructor initializes state11 | 21 |
| 5 | noexcept destructor with inlined ordered cleanup | 110 |
| 6 | Destructor EH handler | 29 |
| 7 | Explicit normal close | 57 |
| 9 | Open/first iteration/empty cleanup/live-pair return | 127 |

There are 13 physical sections and 344 code bytes. Open's actual constructor
calls are +09h and +1Ch, with state12 stored at +12h before the second call.
State13 at +24h precedes genuine first iteration +2Fh; the actual key alone
is passed to the predicate +35h. The nonempty branch +3Fh->7Ah returns true
without cleanup. The empty path lowers13->12 at +49h before value destructor
+53h, then12->11 at +63h before key destructor +6Dh. Explicit close stores12
at +0Bh before value cleanup +15h, then11 at +25h before key cleanup +2Fh.
The destructor has the same order. Open relies on the caller's preexisting
owner for ordinary unwinding; it introduces no private native frame claim.

Destructor .xdata physical12 has magic19930522h, maxState0, flags5 and empty
maps. .sxdata physical11 names symbol33, the section6 handler. There are no
literal key strings or invented game globals; the weak AVX2 bss name in
physical3 is compiler/header metadata.

All four candidate project externals resolve to genuine fresh provider
definitions: constructor34/32 bytes, destructor38/39, predicate60/15, and
protected first62/108. The entire selected callback/protection/finish/publication/
release Source chain was inspected: physical40,41,71,72,73,74,75. Compiled
callback41 inlines start75, finish40 inlines publication72, and first62 inlines
release73; the report records the actual relocation targets. All 66 provider
code sections, 4,240 bytes, and indexed relocations match the retained Damage
entry receipt. This is actual Source-provider evidence, not new Native child
byte identity or validation of every unrelated provider in the TU.

The normal `./scripts/build.ps1` baseline includes registered Damage entry and
excludes this candidate. It passed reconstructed_math (0.22s) and tool_tests
(7.62s), total7.87s. No new tests, candidate fixture, differential or runtime
execution were added. The prior Root Source601 primary review records601
selected inputs,74 Core+3 App whole objects,144 positive Core definitions and
three checks; those are prior context and do not admit this new fragment.

The worker changes only the four owned Source/doc/report files. Shared CMake,
ledgers and Ghidra remain untouched. All source/docs, actual provider inputs,
retained parent/ABI contracts, prior Root review, baseline inputs and local
COFF/live-listing receipts are pinned. Full-loop/parent composition, Native
ABI/FH3/SEH/longjmp, receiver binding, startup/Present/gameplay and primary
registration/admission remain held.
