# Registered effect type 4 lifetime

Addresses: `008744A0`, `00874540`, `00868D70`, `00874610`.

`NativeRegisteredType4EffectStorage` is the actual 3Ch owner allocated by
`868D70`. Its constructor registers that exact address in the existing live
manager's `references_1c` array. Its destructor unlinks/releases current `+34`,
releases current `+38`, unregisters that address, and restores base identities.
The borrowed `LiveEffectEventRegistryBindings` use the application's same
`F8765C` manager, `F87654` deletion lock and `01090AA0` lifetime domain. No
second registry, count or node domain is introduced.

| Routine | Native ABI and inclusive body | Coverage |
|---|---|---|
| `868D70` factory | ECX definition; stack subject; EAX owner/null; `868DBF`/`868DD3 RET4`; `868D70..868DD5` | complete |
| `8744A0` constructor | ECX owner; stack definition, subject; EAX owner; `87453B RET8`; `8744A0..87453D` | complete |
| `874540` destructor | ECX owner; no stack argument; `8745E3 RET`; `874540..8745E3` | complete |
| `874610` scalar destructor | ECX owner; stack flags DWORD, low bit consumed; EAX original address; `87462B RET4`; `874610..87462D` | complete |

All names are descriptive hypotheses, not recovered class symbols. These are
new C++ interfaces with native owner storage; original MSVC vtable and SEH ABI
replacement and game validation are not claimed.

## Producer-established storage

The canonical `NativeRegisteredEffectPrefixStorage` is embedded at `+00`.
Constructor stores are: `+00=CEB130`, actual atomic `+04=1`, `+08=D0C8C0`,
`+0C=1`, borrowed subject `+10`, borrowed definition `+14`, then concrete tables
`+00=D0DE50`, `+08=D0DE4C`, `+18=4`. The concrete tail is `+1C=1`, untouched
`+20`, zero floats at `+24/+28/+30/+2C` in that order, null node `+34`, null
refcounted owner `+38`. Padding `+0D..+0F` stays untouched. The factory's
`868D87 PUSH 3Ch` and `868D90 ADD ESP,4` establish the allocation extent and
allocator argument count. Constructor ESI is captured from ECX at `8744BF`;
factory ESI is captured from ECX at `868D89` and pushed as definition at
`868DA8`. There is no input owner retain, initialization of `+20`, or extra
event reference count.

## Callee contracts and ordering

| Containing function / site | Native callee | Established operation and implementation |
|---|---|---|
| `868D70 / 868D8B` | `BF681B` | Existing CRT allocation/new-handler boundary; actual 3Ch |
| `868D70 / 868DAB` | `8744A0` | Construct allocated owner with borrowed definition and subject |
| `8744A0 / 87451E` | `4D1100` | Existing locked lazy actual manager singleton; no native arguments |
| `8744A0 / 874525` | `866A10` | Append raw event without retain under the existing deletion lock |
| `874540 / 87457A` | `B6DFA0` | Existing parent/root unlink followed by required current node virtual18 |
| `874540 / 874591` | imported `InterlockedDecrement` | Decrement captured `+38` owner's actual atomic at `+04` |
| `874540 / 8745A1` | captured owner virtual00 | Resolve canonical actual owner only when actual count becomes zero |
| `874540 / 8745AB` | `4D1100` | Reload same manager singleton after callbacks |
| `874540 / 8745B2` | `866B00` | Remove first raw event match by last-element replacement; no release |
| `874540 / 8745CE` | `BD30F0` | Publish `CEB130`; no count or borrowed-field writes |
| `874610 / 874613` | `874540` | Complete direct destructor before physical free |
| `874610 / 874620` | `BF65AC` | Ordinary free iff flags bit0; `874625 ADD ESP,4` |

The node runtime is the existing `GeneratedModelLifetimeRuntime`, using its
actual key lookup. Missing nonnull node bindings fail explicitly. `B6DFA0`
acts on the same bound hierarchy and real virtual18 implementation; there is
no fallback terminal. The destructor clears `+34` only after node cleanup,
then reloads `+38`, decrements its actual count through the existing
`release_native_render_actual_owner`, and clears it only after terminal reentry.
Canonical terminal callbacks are nonthrowing and must dispatch the current
actual class. Valid bindings/storage must survive each native last access.

Constructor and destructor state0 unwind maps are respectively
`DC847C -> C963A0 -> 858150` and `DC84A8 -> C963C0 -> 858150`. Both restore
`D0C88C/D0C888` then `CEB130`, with no unregister, node retry or rollback of
unvisited fields. The factory map `DC7090 -> C951C0` frees the allocation
after constructor unwind. The C++ implementation preserves those boundaries.

All saved direct call sites to the constructor and destructor are the factory
and scalar rows above. The scalar is referenced by table `D0DE54`; the factory
is referenced by definition table `D0D5EC`. There are no additional saved
direct factory callers. The table `D0DE50` begins `BD30E0,874610`; its current
phase-2 virtual28 is `872790`, and virtual30 is `872060`.

## Validation and remaining boundary

Strict MSVC Win32 compilation passed with `/std:c++17 /O2 /W4 /WX /fp:strict`.
The ignored `local/registered_type4_probe.cpp` passed using actual construction,
the real singleton domain and OS deletion lock, the actual raw manager array,
and a real `NativeRenderContextReference` at `+38`. It checked a nonterminal
decrement without owner lookup, terminal destruction while the slot and
registry entry are still present, slot clear after callback mutation, unchanged
borrowed/count/padding fields, scalar flags2/1, and real manager/lock shutdown.
It does not exercise a nonnull `+34`, throwing unwind or original machine-code
execution of these four routines. Seed verification and both existing CTests
passed; the latter are math regression checks, not type4 differential proof.
The integrator owns full CMake registration/build and Ghidra annotation.

These four lifetime routines are complete, but the event family is incomplete.
Actual type4 phase-2 `+28=872790` (`872790..872BC0`) and deactivation
`+30=872060` (`872060..872073`) need their real behavior before family frame
dispatch can be enabled. No successful unsupported update/deactivation stub
or rumble companion registration is supplied. There is no reachable game-host
binding for this new source, so no frame behavior or gameplay result is claimed.

Ghidra analysis was read-only. The scalar body omits `874625..874627` from its
listing after the incorrectly no-return free call: live bytes `83 C4 04` are
`ADD ESP,4`; the existing body continues at `874628` and ends at `87462D`.
`872060` is not a saved function; its last instruction is `872073 RET`, length1.
These are reported to the integrator; no worker flow repair or annotation ran.

## AI integration correction and additional verification

The combined strict Win32 build and both existing CTests passed. The primary
integrator restored the `874625` stack cleanup, defined `872060`, saved recovered
signatures/evidence, preserved the compiler scalar-destructor name, and refreshed
exports. The report retains the worker's original observations and mutation audit.

The extended fixture now also exercises nonnull `+34` with a real pooled174h
node, canonical `NativePlainNodeReference`, actual string storage and existing
node bindings. With the same node at `+34` and `+38`, actual references pass
`2 -> 1 -> 0`. Logical release marks node byte44, then `+34` clears before the
`+38` terminal lookup. The registry entry and `+38` identity remain present during
canonical physical destruction, unbinding and pool return; only then does `+38`
clear and the event unregister. The returned pool slot was immediately reusable.
Both context and node cases passed against the combined library in
`local/registered_type4_probe_merged_ai.log`.

This plain-node fixture establishes the existing lifetime-interface ordering.
The later inspected `BAD6F0` producer identifies the phase2 owner as a
generated-model-derived7ACh `SkinedWaterTracer` with table `D63FA0`. Its real
construction/update, original type4 machine-code execution, throwing unwind and
gameplay remain unvalidated. `872060` and its47-byte `BA9820` callee form a small
next deactivation packet over borrowed actual tracer storage.

## Correction from docs/REGISTERED_TYPE4_EFFECT_BEHAVIOR.md

AJ reconstructs completion872020, deactivation872060/BA9820, all eleven direct
tracer setters and the complete872790 entry control/state behavior through required
real tracer bindings. TRACER_PARAMETER_CURVE.md adds the actual1Ch curve initialized
state and BA9DA0 evaluator, preserving native x87/cache behavior. Direct MOVSS setters
preserve signaling-NaN bits; reciprocalBA9900 and the update caller's x87 argument
spills have separate rounding/quieting behavior.

The combined strict Win32 build, both existing CTests and focused original-byte
fixtures pass. Type4 update coverage includes13 nontracer branches and a required
constructor failure boundary that returns raw storage while retaining the actual
outer lock/depth. It does not establish successful BAC660/BAD6F0 construction,
BAABB0 update, BAA510 predicate, native EH dispatch or gameplay. Actual derived-owner
lifetime and current virtual family composition remain required. The earlier
plain-node lifetime fixture remains limited to the ordering it actually exercised.
