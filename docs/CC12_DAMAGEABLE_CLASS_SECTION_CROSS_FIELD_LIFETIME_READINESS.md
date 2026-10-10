# DamageableClass section cross-field lifetime readiness

The smallest complete normal lifetime of the MshCategory string is
`[0087CEBB,0087D1A0)`: **741 bytes / 210 instructions**. It includes Index,
category publication, both FireEfx paths, FailureChance, FailureDamageThreshold
and the matched string release. That body is **held**: the current Source search
still has no implementation of the called `00870CD0` effect-by-ID wrapper, and
the existing effect APIs retain explicit type, manager and application-service
boundaries. No full-body Source packet is proposed.

The separate `[0087D1A0,0087D1CF)` iterator tail is **47 bytes / 12 instructions**.
Its current genuine Source provider graph is closed under the existing state13
owner contract, so it is an independent ordinary, unbound future Source proposal.
This readiness adds only this document and its JSON receipt; neither proposal
is implemented here.

## Frozen evidence and scope

Root supplied a frozen whole-parent buffer, complete decoded index and accepted
typed metadata at modification10. All four files, including their receipt, were
verified and copied to `local/section_cross_field_retained/`. The 3,238-byte buffer
hash is `00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
Every one of its 858 decoded instructions matches the frozen index and every
start matches the historical parent receipt. The typed response's original raw
bytes/hash are verified; its complete single body is CA80..D725, returning and
non-thunk. Typed function metadata is not a proof of the original register ABI.

The exact authorized cap `[0087CEBB,0087D1CF)` is 788 bytes / 222 instructions,
SHA-256 `74d1326bca0e448448486d60f8684141fbc6ecc82ba4d2487d8805988f7ea214`.
All 19 conditional branches and 38 calls (28 direct / 10 indirect) are indexed.
The cap was leased before focused reading. No installed PE, current Ghidra,
new Native callee/data/name/table/handler/body or out-of-cap code was opened.
The prior first-field report remains unchanged. Its earlier raw-byte limitation
is historical; this separate packet now has Root's later frozen buffer.

| Region, exclusive end | Bytes / instructions |
| --- | --- |
| CEBB..CF35: MshCategory string construction/value cleanup | 122 / 35 |
| CF35..CF70: Index | 59 / 13 |
| CF70..CF96: category publication and FireEfx lookup | 38 / 11 |
| CF96..D031: effect ID, assignment and two cleanups | 155 / 48 |
| D031..D0FB: optional named fallback and cleanups | 202 / 62 |
| D0FB..D141: FailureChance | 70 / 16 |
| D141..D181: FailureDamageThreshold | 64 / 15 |
| D181..D1A0: MshCategory normal cleanup | 31 / 10 |
| D1A0..D1CF: next iteration and key predicate | 47 / 12 |

## Actual storage and the retained string

S is ESP after the parent's E4h locals/four saves. The actual selected row stays
in ESI throughout the field body. It is never reselected from a possibly changed
vector header or S+DCh. The body writes row+8 (Index), +4 (category), +24h (effect),
+28h (chance) and +2Ch (threshold). It does not allocate another row or call the
vector's invalid-parameter service. Row validity remains a caller/storage
precondition; no bounds repair is introduced after the earlier returning checks.

Existing state13 owns actual key S+58h/value S+2Ch. Sections S+80h, Damage S+44h
and Unique S+94h remain outer owners. The MshCategory field uses fresh S+E0h;
later field lookups reuse S+6Ch. Actual strings are S+18h (MshCategory) and S+10h
(fallback name); effect handle temporaries are one-word slots S+28h and S+20h.
These are distinct actual storage identities, not copies or new invented types.

The first 122 bytes retain the earlier scan/resize/copy schedule: direct converting
Lua getter, raw string zeroing after it returns, bytewise scan to the first NUL,
resize with preserve=true, current data capture, unconditional current length
capture after TEST, and nonnull BF7680 even when length+1 wraps to zero. The
generic Source constructor does not prove those exact operations.

At CF07/CF0D, **EBX captures the data pointer and EBP the length**. Category lookup
uses captured EBX, substituting retained empty-string identity F878E0 only when
that captured pointer is null. The final normal release also uses those captured
values, even if later services change the S+18h header. It tests EBX, lowers17->13
at D183, and only then obtains the current pool and returns EBX with EBP+1.
The header is not cleared and the release is not retried on failure.

In contrast, retained unwind state17 invokes 41DD20 on the **current S+18h header**.
A generic header destructor at normal D181 would therefore lose the normal/EH
capture distinction. Native normal EBP is the captured length; the earlier
funclet's EBP-relative storage notation belongs to its established EH frame.

## Effects: ownership, current values and all normal branches

The value at Index is read through B66270, converted through BF7420 and stored
to row+8 before lowering18->17 and releasing its Lua object. Category lookup then
consumes the captured name, and row+4 is published before the FireEfx lookup call.

At CF96, PUSH1 prepares the **later effect wrapper's flag**. PUSH0 supplies
B66380's fallback. Its existing RET4 contract consumes only fallback0, leaving
flag1 pending: CFAF receives EDX=integer ID and ECX=actual output S+28h. The
Source integer-conversion mode parameter is a separate provider input; it must
not be replaced with that pending literal1. The wrapper's precise implementation
and register ABI remain unreviewed; it was not opened to fill the missing edge.

Both acquisitions load the new owner through the provider's returned address,
capture current row+24h as the old owner, compare them, and only then enter the
temporary handle's state20 or22. Equal pointers skip assignment and both row
retain/release operations. Otherwise publish the new pointer **before** retaining
it, then release the captured old owner. A zero final count dispatches its
current vtable slot0 with only the actual receiver and no deleting flags.
Callback writes to row+24h are preserved; no post-callback assignment repeats.

Each temporary cleanup rereads its current slot after row-assignment callbacks.
It captures/tests that value before lowering20->19 or22->21, then decrements the
captured owner and calls its current slot0 on zero. Only a nonnull path that
returns normally clears the original slot, including any callback replacement.
A null path does not write the slot. A throwing callback prevents the clear and
does not cause a retry after the state was lowered.

After first temporary and Lua cleanup, D031 rereads **current** row+24h. The
NEG/SBB/TEST-immediate sequence is exactly a nonnull test: E17BF8 is an immediate
mask here, not a dereferenced global or extra effect-provider input. A nonnull
row skips the fallback. Null constructs actual S+10h from retained `BigFire`,
then calls the existing name acquisition with actual S+20h output and flag1.
Fallback string cleanup captures current data before lowering21->17; only its
nonnull path reads current length and obtains the pool. Neither string header
is cleared by normal cleanup.

All conditional transfers are retained explicitly:

| Site(s) | Taken branch condition |
| --- | --- |
| CEF7 | Scanned byte is nonzero: continue scan. |
| CF11 | Current resized data is null: skip copy. |
| CF74 | Captured category data is nonnull: skip empty-name substitution. |
| CFC3 / D077 | Old and new row owners are equal: skip row assignment. |
| CFCA / D07E | New owner is null: skip retain. |
| CFD8 / D08C | Captured old owner is null: skip release. |
| CFE6 / D09A | Old-owner decrement is nonzero: skip virtual call. |
| CFFE / D0B2 | Current temporary slot is null: skip release and slot clear. |
| D00E / D0C2 | Temporary decrement is nonzero: skip virtual call, then clear. |
| D03D | Current row+24h is nonnull: skip all fallback work. |
| D0E2 | Captured fallback string data is null: skip pool return. |
| D18B | Captured MshCategory data is null: skip pool return. |
| D1C9 | Key is still bound: transfer to accepted CE00 predecessor; otherwise fall through to accepted D1CF teardown. |

## States and x87 ordering

The retained 37-state map is used as metadata; the state stores below are also
verified in the frozen ordinary body. State16 exists in the map but has no
ordinary activation in this cap; do not invent it as a scan/resize/copy guard.

| State | Parent | Actual cleanup |
| --- | --- | --- |
| 15 | 13 | Lua field S+E0h |
| 16 | 15 | MshCategory string S+18h |
| 17 | 13 | MshCategory string S+18h |
| 18 / 19 / 23 / 24 | 17 | Reused Lua field S+6Ch |
| 20 | 19 | Effect temporary S+28h |
| 21 | 17 | Fallback string S+10h |
| 22 | 21 | Effect temporary S+20h |

Construction/acquisition must finish before entering its new state. Primary
string scan/resize/copy runs in15. The first Lua cleanup runs after entering17.
Effect acquisition by ID runs in19, and by name in21; their output dereference,
old-row capture and comparison also precede states20/22. All normal cleanup
states are lowered before invoking their provider, preserving which cleanup
is eligible if it fails. Existing outer13->12->11->10->1 ownership is unchanged.

Index's B66270 result is binary32 in ST0 and goes directly to the mode-dependent
BF7420 converter; no extra double/float expression or C++ cast is substituted.
Current Source B66270 explicitly narrows Lua's double through x87. Its direct
number getter can coerce a numeric string; FireEfx's B66380 and the two defaulted
float fields instead require an exact Lua NUMBER or use their fallback.

FailureChance loads retained float32 -100 from CE65D8 and spills its stack
fallback **before state23**, then calls B66330. FDIV uses the retained binary64
100 cell D7A220 while state23 is active. D131 lowers23->17 **before** the FSTP
to row+28h at D139 and before Lua cleanup. The final float store can therefore
fault with a different active cleanup chain from the preceding division.

FailureDamageThreshold likewise loads/spills retained float32 -1 from D7A260
before state24. Its final FSTP to row+2Ch at D16D occurs **under state24**;
D174 lowers24->17 only afterward. A generic float-field helper with one common
store/cleanup order would miss this distinction.

Actual x87 control word, precision/rounding, status, pending exceptions and ST0
consumption matter. A binary64 division operand is not permission to insert a
binary64 intermediate spill; final binary32 stores and their state positions
must remain explicit. Current BF7420 Source reads an actual mutable dword mode
address at conversion time and selects FSTP-double/CVTTSD2SI or complete BF7456.
Its inherited report's older `gui_group_bounds.cpp` location is stale; the real
body now lives in `native_render_batch_keys.cpp`. Lua integer-or has an explicit
bool mode Source contract. Neither is proof of original Native ABI/fault parity.
Constant bits here come from retained metadata; no Native constant cell was read.

## Current Source and the held dependency frontier

The receipt freezes complete relevant Source files, the project header closure,
current Lua dependency files, ten full bounded searches and baseline pins. Real
Lua lookup/getter/cleanup/next-iteration bodies, raw string resize/cleanup and
actual pool bodies were checked. The current intrusive release helper has the
correct captured-owner/current-vslot/clear-after-return behavior, but invoking
it after lowering state would not by itself reproduce an earlier capture/test.

The category provider uses the actual borrowed volatile table and current
`compare_insensitive_00438e10`; its host CRT/locale boundary and concrete table
binding remain explicit. No fixed category enum or guessed table is introduced.

The smallest concrete missing edge is `[CF96,CFB4)`, 30 bytes / 8 instructions:
integer fallback setup through the return from `00870CD0`. Current src/include
address-matched search finds only the historical dynamics-interface comment for
that wrapper. It does find the genuine lower `008700E0` acquisition and existing
`00871BA0` name wrapper. This is a bounded implementation lookup, not a claim
that no arbitrarily named equivalent could exist.

The lower effect bodies are qualified current Source, not a replacement wrapper:
they use a canonical host map, current manager/context, game Lua provider and
component services. The name API takes `NativeString const&` and `void*&`; raw
S+10h/S+20h bytes do not by themselves establish those C++ object lifetimes.
Actual type/storage bridges, process name-index binding and application services
must be real. Even default ID0 must not become an invented fast null path that
skips the missing wrapper's possible manager work. No new Native child was opened
and no synthetic callback/provider/type is supplied to close these edges.

The full 741-byte future body remains held. It needs that genuine wrapper and
the explicit provider/binding/type frontier, plus an owner that preserves normal
captured string cleanup versus current-header EH cleanup and every state/store
schedule above. Its exact prospective normal boundary is CEBB..D19F, not CF34,
D183, or an earlier point where the string is still owned.

## Independent future iterator-tail contract

Proposed Source scope: `[0087D1A0,0087D1CF)` only, 47 bytes / 12 instructions,
SHA-256 `7ace1a66d9c3f246cda60da0bf50fe88192baa7ced611137a71a8418ee781865`.
Borrow the successful existing iterator owner and its same actual scratch,
Sections S+80h, key S+58h and value S+2Ch. Enter only after all inner owners have
normally closed to13. Keep the pair exclusively owned and existing Lua tracking,
capacity, stable identity and inherited error-handler contracts valid.

Call the genuine existing `native_lua_iterate_next_protected` on those objects,
then return `!native_lua_is_unbound_00b66420(actual_key)`. Native B66420 ignores
ECX/Sections and examines its stacked key, not the value or table. The current
Source bodies are complete: next releases the old value, detaches/copies the
actual key correctly, protects Lua next and registers the actual returned pair.
Failure retains completed releases/index shifts and discards the detached work
slot; it does not restore the old pair. Ordinary enclosing owner cleanup handles
propagation. There is no new field owner, row mutation or effect/string service.

True corresponds to the excluded CE00 back edge. False leaves the still-existing
pair owner for the already accepted D1CF teardown; do not close it early inside
this tail, call `open` again, copy keys/indices or create another iterator.
No new Native loop/body or whole-parent claim follows from this proposal.
Registration, compilation and whole-object review would belong to a separately
authorized Source packet; none occurred here.

Root Source632's 632 inputs, 87 Core / 3 App objects, 227 definitions and three
existing checks are pinned context only. No C++, CMake, providers, ledgers, GPR,
build, test or probe changed. Production composition, Native ABI/FH3/SEH/longjmp,
fault identity and new startup/gameplay execution remain held.
