# Complete ordinary health setter sequence

Packet `cc11_health_setter_sequence` replaces the precomputed snapshot path in
`unit_set_health_00877b90` with a required concrete binding to
`set_native_unit_health_00877b90`. The complete ordinary caller now writes health,
runs the health-changed hook, and only then observes the state needed for the
getter and replication. A callback can change the health, maximum, release flag,
session, virtual table or replicated byte, including through a nested setter.
Descriptive names are hypotheses, not recovered symbols.

The separately contracted `set_health_00877b90` data projection remains intact.
Its supplied session/release/byte snapshots cannot represent these callback
effects and no longer drive the `UnitDamageHost` setter wrapper.

## Native ordering and source contract

Read-only live Ghidra batches verified project `bsp`, program
`/battlestationspacific.exe`, x86 LE32 and image base `00400000`; the configured
project is `C:/Users/sqz269/bsp.gpr`. Full listings of `00877B90` and `00923BE0`,
plus constructor/router boundary inspection, establish:

| Native site | Required ordinary behavior |
| --- | --- |
| `00877BAB..00877BC1` | Ordered equality returns before clamp, store or session observation; opposite signed zeros compare equal. |
| `00877BC7..00877C04` | Clamp the request using the evidenced ordered comparisons, preserving masked unordered storage behavior, then write actual `+370h`. |
| `00877C0C..00877C20` | Reload maximum, run the proved ambient-x87 marker predicate and only set `+2E8h` to `FFFFFFFFh` when true. |
| `00877C2A..00877C40` | Read the current game pointer/mode; exactly mode 2 skips the callback. Otherwise load current table `+1B0h` and call it. |
| `00877C42..00877C53` | Reload both game pointer and mode after the callback; exactly mode 1 enters the getter. Other values return. |
| `00923BE4..00923BF9` | Read actual `+5Dh` once. Nonzero returns positive zero without table/provider/cache work. Otherwise resolve current table `+110h`, call the provider and spill its result to binary32. |
| `00923BFD..00923C49` | Run the shared native floor/cap suffix, load return ST0 before storing actual `+164h`. Do not retest release or session after the provider. |
| `00877C58..00877C77` | Use the proved mode-dependent numeric byte kernel, then compare actual current `+374h`. Provider/CRT effects precede this observation. |
| `00877C84..00877C9E` | Store the new byte before constructing the message. Pass constructor-returned identity, receiver, flags 4 and null final argument to the router. No later setter read/store restores pre-constructor state. |

The shared getter suffix retains `FLD` before cache `MOVSS` on both the lower
floor and ordinary/cap paths, matching `00923C12/C16` and `00923C3D/C41`.
The previous numeric-only getter reuses this suffix with its own spill as the
destination, preserving its existing no-entity-cache contract. The complete
caller supplies the actual cache cell. Existing marker, fraction and CRT byte
kernels remain shared; no converter or CPU policy is duplicated.

`NativeUnitHealthSetterFields` borrows actual receiver cells, and
`NativeUnitHealthSetterGlobals` borrows the mutable game-pointer cell plus the
actual readable `0109EEA4` address. Volatile accesses retain the required live
observations. Primary-table and session-field accessors are pure, nonthrowing
views of actual backing; they cannot allocate, callback, snapshot state or
change FP controls. Dispatch entries are owning-runtime tokens, not original
addresses cast to executable pointers.

All callback, provider, constructor and router operations are required complete
bindings with no default/no-op implementation. Their effects may include
reentrancy and changes to the actual cells. Reached identities/cells must remain
live, as the original subsequent accesses require; there is no added release,
death or reentry guard. Unsynchronized concurrent writes and invalid receiver
lifetime are outside this ordinary-return contract.

## Message and ABI boundaries

The native caller reserves exactly `20h` bytes, passes that stack storage to
`00876D30`, then uses its returned EAX identity. The source likewise supplies an
uninitialized, four-byte-aligned `NativeUnitHealthMessageFrame` of exactly 32
bytes. It supplies no profile, arena, allocator or invented owner.

The required constructor owns the complete D2 construction contract: base
`0075B430`, current-game selected owner, sender/relay initialization, payload at
`+1Ch`, delivery word 1 and the real derived profile. Native `00876D30` returns
its construction identity. The required router owns all its own current-state
checks, routing, cloning/transport and ownership behavior. The caller frame
remains live through route return and cannot be retained afterward. In
particular, a provider or constructor changing the session to client does not
add a new setter gate; the router receives the call and applies its own rules.

This is a new C++ source ABI. Original setter ECX/RET4, private EH/unwind state,
fault timing and native register ABI are not supplied. The provider interface
admits a float return spilled to binary32 before the proved suffix; arbitrary
extended ST0-return equivalence is not claimed. The adapter also spills the
getter result before the byte kernel. Returning masked FP data/control behavior
is tested; full status/fault equivalence and unmasked resumption are not.
Kernels do not install control policy. Required calls execute their real effects,
including any permitted changes to floating-point controls.

## Existing host inspection

An exact inheritance search found no `UnitDamageHost` implementation in current
`src`, `include` or `tests`. The new pure binding accessor is appended to that
interface, retaining the order of its existing virtuals. Its wrapper now uses
the complete bound caller; `unit_apply_damage_00879070` still reaches that wrapper.

The five direct projection sites in `game_hosts_gunnery.cpp` explicitly use
campaign snapshots and separate health/death policy. They do not provide the
actual released/cache/session/message ownership contract. `GameScriptOrdersHost`
also exposes a mirrored health provider and a no-op cache-store service. Those
services are not admitted as bindings for this caller. Their separate existing
projections and runtime policy remain unchanged; no fake cache or CRT state is
wired into the game hosts.

## Focused evidence

The existing local health probe was extended, with no tracked test suite added.
It compiles and links the actual production sources using MSVC Win32
`/O2 /W4 /WX /fp:strict`, with an embedded `asInvoker` manifest.

All 336 comparisons of the new bound prefix against the native instruction
fixture pass across PC24/53/64 and four rounding modes, including the existing
NaN/infinity/signed-zero cases. These checks also preserve two live x87 values,
TOP, CW and MXCSR controls. Eleven full caller cases pass, each with the same
live-stack/control checks:

| Fixture effect | Observed result |
| --- | --- |
| Campaign callback writes health 50/max 200 and replaces game pointer with host | Marker already set before callback; fresh getter/cache is 0.25 and message byte 64. |
| Callback changes session to client | No getter, byte store or message construction. |
| Callback releases receiver | Provider/cache skipped; byte zero constructed/routed. |
| Provider changes release, session, cache and stored byte to the new result | Cache still updated after provider; fresh byte comparison suppresses the message. |
| Provider changes session to client but leaves a different stored byte | Caller still constructs/routes byte 128; router observes client state. |
| Callback recursively sets health to 25 | Two hooks/getters, one message; outer call observes nested health/cache/byte. |
| Constructor changes health/max/release/session and byte to 199 | Router receives original payload 128 and sees byte 199; setter does not restore it. |
| Initial client, mode 3, table replacement and unchanged signed zero | Respectively: store-only; callback-only; fresh provider selection; no session/callback work. |

Fixture providers/constructor/router are explicit instrumented operations;
their receipts prove caller sequencing, not native transport execution. Native
prefix comparison likewise executes an instruction transcription, not the game.
Prior checks remain clean: 336 prefix, 672 session, 240 prior byte, 336 state,
192 each fraction/getter/released, 576 numeric state, 624 CRT comparisons and 48
same-address mode reloads. Generated assembly was checked for fresh reads,
provider spill, cache ordering, byte store before constructor and returned
message identity forwarded to flags-4/null routing.

See `reports/unit_health_setter_sequence_cc11.json` for hashes, receipts and
direct-call verification. Reproduce with
`local/cc11_health_setter_sequence_check.cmd` in the worker worktree. Full build,
metadata/annotation and integration belong to the primary. Actual game bindings,
native ABI replacement and gameplay validation remain unestablished.
