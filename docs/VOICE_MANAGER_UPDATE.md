# Voice manager update and attached playback

Packet `orch4_voice_manager_update_c` plus supporting lease
`orch4_voice_attached_helpers_c`, branch `agent/orch4-voice-update-20260910c`,
2026-09-11 UTC. All live queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Worker Ghidra access was read-only. Names below
are descriptive hypotheses, not recovered symbols; annotations are pending
the integrator's write lock. Original names were `FUN_<address>` and the live
comment query returned no prior plate comments for these five functions.

## Scope and native ABI

| Address / proposed name | Native interface | Last instruction / length | Scope |
| --- | --- | --- | --- |
| `005BC640` / `BSP_VoiceManager_Update` | ECX=manager; float delta stack; activity DWORD EAX; RET4 | `005BC916` / 3 | Complete normal-path manager loops, including missing continuations |
| `005B7290` / `BSP_AttachedVoice_Update` | ECX=entry; bool AL; RET | `005B7385` / 1 | Poll, spatial attenuation, volume or stop |
| `00702130` / `BSP_VoiceSlot_SetVolume` | ECX=slot; float stack; RET4 | `007021BC` / 3 | Auxiliary then state-selected primary/named volume |
| `007026F0` / `BSP_VoiceSlot_Stop` | ECX=slot; RET | `007027A2` / 1 | Stop/release with callback-sensitive reloads |
| `00701870` / `BSP_AlternateVoice_SetNamedVolume` | ECX=alternate engine; NativeString pointer, float stack; RET8 | `0070189B` / 3 | Exactly two named-channel calls |

Semantic implementations are in `src/voice_manager_update.cpp`, interfaces in
`include/bsp/voice_manager_update.hpp`. They use canonical `VoiceLine`,
`VoicePlaybackManager`, `VoicePlaybackSlot`, `NativeString`, GUI transforms,
existing line advancement, slot polling, and embedded-slot destruction. They
are not binary-layout or native-ABI replacements. Native SEH exceptional
cleanup, exact x87 extended precision, signaling-NaN quieting/payloads,
unmasked traps and FP status/control equivalence are outside this scope.

## Two different list layouts

The main queue is existing manager `+54` count, `+58` first, `+5C` last;
its nodes have previous `+0`, next `+4`, line pointer `+8`.

The additional queue is an STL-shaped sentinel list: manager `+64` stateless
allocator, `+68` sentinel pointer, **`+6C` count**. Its nodes have next `+0`,
previous `+4`, entry pointer `+8`. These link directions differ from the main
queue. `VoiceAttachedQueueView` aliases the real sentinel/count storage; it
must not create a second count beside the field previously named `blocked_6c`.

`VoiceAttachedEntry` projects the embedded 18h playback slot at `+0` and a
callback-owner base at `+18`. The latter's vtable is stamped `00CE74FC` during
deletion (`005BC88B`); entity is base `+14`, hence entry `+2C` (`005BC891`).
The unknown observer internals stay with the actual owner through required
adapters. The struct is a semantic projection, not an invented native layout.

## Main queue, expiry, and relayout

For each node, `005BC670` caches its line. A nonzero target is invalid when it
matches current `00E188D8` or `00645160(target,true)` returns false. The code
loads shortcut widget `+34` before clearing target `+2C`, then calls widget
virtual `+34(false)` without inventing a null check.

The timer test `005BC6A3 COMISS(0,line+24) / JC` treats positive **or NaN**
hold duration as active. Otherwise existing `advance_voice_line_005b91e0`
decides activity. Either active path subtracts delta from the current hold
duration. The inactive path subtracts delta from linger `+20`, spills float,
then deletes if ordered linger<=0 or widget `+14` is null. NaN linger with a
nonnull widget survives. No timer or delta normalization is added.

Deletion unlinks both directions, captures successor before destruction,
decrements count, reloads node payload, and invokes virtual scalar deletion
with flag1. The exact line-deletion host remains the actual virtual-dispatch
contract. Parent will bind the sibling's `scalar_delete_voice_line_005b9b80`
for canonical vtable `00CF0ED4`, retaining that virtual contract for other
runtime vtables. Node free follows. The
recovered continuation uses the captured successor and sets dirty `+60`
**after** destruction/free. Surviving nodes instead reload their next link
after callbacks, matching `005BC72B`.

When dirty, read first node then clear dirty and start baseline at +0. For
each nonnull widget, ordered Y>baseline invokes the `00432650` getter and
reads its returned owner `+80`; reload the node's line before subtracting.
Then `005BC794 FCOMIP(baseline,Y) / JC` leaves Y and sets dirty if baseline<Y
**or unordered**; otherwise clamp Y to baseline. NaN Y therefore keeps dirty.
Add the existing `widget_size` height to the current Y and spill the next
baseline. Preserve the native position-line identity, retain local x, use
line Y, and reload the destination widget. The `00AA7D00` caller contract is
implemented through the already recovered local transform and bounds calls,
preserving z. Next link is read after GUI work. Primitive getters are
side-effect-free; required native callbacks can change live fields.

## Attached activity and teardown

Capture node successor before calling `005B7290`; reload the sentinel at each
loop comparison. An inactive result unlinks/frees the node and decrements
count **after** free. If callback mutation makes the current node equal the
sentinel, invoke native CRT policy `00BF6713`, recheck, and skip unlink if it
still equals sentinel. The entry cleanup still follows. Tautological debug
iterator checks without intervening callbacks are omitted under the valid
live-list precondition; malformed containers receive no fabricated fallback.

For a nonnull entry: stamp callback-owner vtable, load current entity and call
`006952A0` (ECX=entity, EDX=actual entry+18) if nonnull; then `00695870` on that
same actual subobject, concrete `destroy_voice_slot_005b7fc0`, and entry free.
The native callback-owner cleanup's remaining fields and exception behavior
are not ported. This is distinct from scalar line deletion. Current objects,
loaded owners and captured successors must remain valid through their next
native use; arbitrary destruction of the active iterator by reentry is not a
new supported container policy.

Final activity is line count!=0, attached count!=0, or current
`[00F8A0C4]+E8 !=0`, in short-circuit order. The final global owner is queried
only when both counts are zero. Scheduled warning rows in parent-owned
`005BBF10` are not duplicated here.

## Spatial and audio helpers

`005B7290` first calls existing `poll_voice_slot_007027b0`; zero returns false
without another stop. For an active entry with entity, refresh/read current
camera via the existing camera contract, then **reload the entity field**
before its dirty-position refresh/read. Native has no second null guard.
Subtract three float coordinates with float spills, reuse the vector-length
contract `0042B2F0`, compute `(2000-distance)/2000`, and spill to float. Live
constants: `00CF0DD8` double2000, `00CE3868` float0.25, `00D7A218` +0float.
Admit attenuation>=0.25 and attenuation>0; otherwise stop and return false.
For ordinary nonnegative finite distances this includes 1500 exactly. NaN
fails the final positive test and stops. The double host length remains a
numerical projection of native ST0, not exact extended-precision proof.

`00702130` writes auxiliary sound volume `+24` then dirty byte `+14` when
auxiliary is nonnull. Strict bound comparisons clamp to [0,1], preserving NaN
and signed zero. State1 applies the same stores to the current main sound,
without a null guard. Every other state, including zero, passes the original
unclamped volume and borrowed slot NativeString to `00701870`. The latter
captures alternate engine's channel-array owner and calls `00A78330` exactly
twice, reading channel2 after channel1's callback. No string copy is added.
`VoiceSoundVolumeView` aliases actual volume and the canonical sound-level
dirty byte; `VoiceAlternateChannelsView` aliases the two captured-owner slots.

`007026F0` clears the current sound-manager flag when initial state!=0. It
stops the auxiliary, reloads and releases its current reference, then clears
the field after release. It reloads state after auxiliary callbacks. State1
stops the current main sound, reloads/releases its current reference, clears
it after release and sets state0. State2 queries current alternate engine;
if playing, reloads that global for `00A78620`, then sets state0. Other states
remain unchanged. New volume/stop primitives have no success/no-op fallback.

## Ghidra flow handoff

Read-only flow audit: 005BC640 has223 listed instructions and4 gaps; helpers
005B7290/00702130/007026F0/00701870 have65/46/78/20 instructions and zero gaps.
Manager bytes checked against raw executable disassembly establish:

| After call | Inclusive missing bytes | Recovered continuation |
| --- | --- | --- |
| `005BC711 _free` | `005BC716..005BC720` (11) | add esp,4; mov esi,edi; dirty60=1; jmp005BC72E |
| `005BC868 _free` | `005BC86D..005BC873` (7) | add esp,4; add [ebp+8],-1 (manager+6C) |
| `005BC8C0 _free` | `005BC8C5..005BC8CC` (8) | add esp,4; jmp005BC7FC |

Integrator should repair these false call-site no-return overrides under the
write lock and refresh exports. Skipped `005BC75A..005BC75F` is six-byte LEA
alignment padding, not missing flow. No owned address lacks a Ghidra function.

## Validation and integration

`./scripts/build.ps1` passed MSVC Win32 Release and existing
`reconstructed_math`1/1. One ignored fixture
`local/voice_manager_reentry_check.cpp` passed through the concrete manager,
slot stop, and named volume implementations. It checks real heap-node
deletion with saved successors, before/after-free count and dirty ordering,
observer/embedded-slot cleanup order, aux/main reference and state reloads,
and changed second-channel lookup with unclamped named input. Unused fixture
services fail on unexpected calls. No shared tests or test targets were added.
This is host-fixture evidence, not native differential, actual FMOD playback,
game validation or GUI rendering proof. The existing helpers retain their
own evidence boundaries. Parent supplies the canonical attached sentinel/count
and binds canonical scalar deletion after lifetime merge, retaining actual
virtual dispatch for other runtime line types.
