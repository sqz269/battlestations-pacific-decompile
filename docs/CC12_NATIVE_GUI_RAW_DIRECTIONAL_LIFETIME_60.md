# Native GUI raw directional lifetime, packet 60

The existing typed `DirectionalLightOwner` / `DirectionalLightReference` graph
now admits an actual raw name through its SAME node runtime and raw string
context. It reuses the recovered raw constructor and existing complete light
destructor. It allocates no second pool, hierarchy, scene registry, native light
or reference counter. This is Source composition; no normal Screen/Text host is
activated, and no new Original factory/FH3 or gameplay proof is claimed.

Base: requested Root `7e8390fecd4bfe6c46e0a9145517263d5a27d738`, merged successfully
at worker `503b26751ccb22057c19302205a2a1971bc92bf4`. Ownership is the four
directional owner/reference files and this unique doc/report. No CMake, process
pool, ScreenString/store/manager, raw-storage lifetime, ledger or Ghidra mutation.

## Original evidence and reuse

Verified the existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`
with `bsp.py ghidra ensure` before the `bsp.py show` evidence batch; reviewed
both pseudocode and the relevant assembly, including the returning free tail.
The function-entry leases cover these existing recovered bodies:

| Original range | ABI / established order |
|---|---|
| `[00B7C4C0,00B7C576)` | 182B, ECX actual owner, stack actual 8h name header, EAX same owner, RET4. B7C4C8 calls Node B6F5A0; B7C4CD reads current D7A24C. Stamps D62F58; zeroes 174/178/17C/180, writes one to 184..190/1A4..1B0, zeroes 1C4/1C8/1CC, then B7C550 reads current CE7820 before 1D0/1D4/1D8 stores. |
| `[00B7C6B0,00B7C6C9)` | 25B, same owner/name/return ABI. B7C6B8 calls base; only then stamps D62FB0. |
| `[00B7C5B0,00B7C6AA)` | 250B, ECX light, RET. Light-phase scene drain reloads live back/count after callbacks, releases shadow174, destroys array178; B7C693 calls complete Node B6F440 and B7C6A9 returns. |
| `[00B7C820,00B7C846)` | 38B, ECX owner, stack flags, EAX original owner, RET4. Calls full light destructor, then low-byte bit0 conditionally returns SAME slot through B7B2F0/01090154. No slot return on a throwing destructor. |

`native_directional_light_construction.cpp:45,57` remains the authoritative
raw constructor kernel; it preserves native-unwritten 194..1A3, 1B4..1C3,
1DC..1EB and pool slab-ID1EC. B6F5A0's existing raw constructor owns its
one-shot prefix cleanup. None of those bodies is duplicated here.

## Source route and failure ownership

`directional_light_owner.hpp:38,41` adds raw-header overloads.
`directional_light_owner.cpp:105,112` calls the existing corresponding raw
kernel, then `materialize_raw_light` (`:67`) snapshots all 78h tail bytes,
starts the trivially copyable typed tail lifetime at SAME +174, and restores
all bytes. The existing Node prefix/atomic already exists. There is no guessed
specular/mode3/direction initialization, no constant snapshot before its native
read, and no write into slab ID1EC. This typed materialization is a Source
lifetime boundary, not a native instruction-count/asynchronous-fault claim.

`finish_node_base` (`directional_light_owner.cpp:51`) now selects the existing
throwing raw Node destructor when `runtime.uses_raw_name_pool()`; otherwise it
keeps its semantic call. Tail lifetime end / destroyed-binding forget remain
in the existing normal and exception paths. The whole light drain/shadow/array
body and semantic constructor/factory are unchanged. Raw release exceptions
are not concealed by a noexcept semantic string adapter. Existing canonical
terminal-reference methods remain their explicit noexcept boundary: a terminal
raw failure terminates rather than returning a partial slot or replaying cleanup.

`DirectionalLightRawAllocation` (`directional_light_reference.hpp:86`) is a
single-use, immovable caller-retained operation frame. Establish it before the
light's first name/pool/native effect. The caller's enclosing persistent frame
retains the actual header bytes and any unconsumed caller name cleanup; this
API borrows its exact address, never copies/mutates/releases that input header.
Its marker does NOT cover a caller's separate temporary-name constructor/release.
Such caller operations need their own persistent entered/consumed state and
fail-stop guard, as already required for the raw page-root route.

The new factory (`directional_light_reference.cpp:164`) first rejects used
frames, missing headers, invalid actual profiles/predicates or mismatched
scene/attachment runtimes, then admits `nodes.require_raw_name_pool()` without
calling a getter. The mandatory constant arguments are borrowed actual
`NativeNodeRawConstants` (CE4970/D7A24C/CE4ADC) and actual CE7820 references;
there is no fallback provider/value. It marks entry before B7BAC0 and retains:

1. Exact header identity and acquired actual 1F0 slot before constructor entry.
2. Constructor-entered / constructor-returned-and-tail-materialized state.
3. The completed owner companion before potentially throwing scene admission.
4. Scene-binding state before potentially throwing canonical-reference admission.

Any entered exception leaves that stage/slot/completed companion in the SAME
frame. There is no catch rollback, unbind, native cleanup, slot return or retry.
A failing C++ companion constructor has not published a completed companion;
its language-level subobject unwind adds no native cleanup. Existing scene and
reference bind implementations only append after validation and successful
vector growth; no callback follows a successful append in these constructors.
B6F5A0 may already have consumed its own prefix cleanup; this caller never
replays it. Allocation throws before returning a slot can still have pool/list
effects, so `pool_entered` is intentionally interrupted as well.

After successful scene and lifetime binding, the factory publishes exactly one
canonical `DirectionalLightReference`, borrowing the actual +4 count1. It
clears frame pointers before marking publication/return; no provider or callback
follows that handoff. Existing terminal disposal captures the owner, deletes
the retired reference, then deletes the owner; no companion access follows.
There is no additional retain or destruction companion. In particular, do not
install `NativeDirectionalLightStorageReference` on this same light: the
separate `native_light_lifetime` pipeline remains unchanged and uses a distinct
raw-scene architecture.

The metadata-only `native_operation_interrupted()` reads no raw storage, locks
or callbacks. It is true while entered and after failure, false only before
entry or after successful publication. There is no reset. External GUI
serialization includes reentry. The frame destructor terminates if interrupted
before discarding its metadata; the future host must inspect its persistent
marker and `_Exit` before any member, manager or CRT drain. Do not unwind an
outer graph first and then inspect a disappeared local frame.

## Supported composition and remaining work

Pass the normal renderer's SAME `camera_environment().nodes`, scene/attachment
runtimes, VFS raw string context and actual `renderer.node_constants()` cells;
borrow actual CE7820 and the immutable directional/light/node tables. Use the
already initialized canonical VFS type predicates. Names/constants must remain
accessible through construction; profiles, pool, runtime/type and raw-name
domains survive every final callback and shared drain. The creator self-reference
must eventually receive logical release; retained queue references can delay
terminal retirement. Return every slot before process pool CE0EB0.

Root's separate packet62 supplies explicit canonical process-pool ownership,
not a reason to initialize or activate it from this packet. Screen caller/name
frames, actual registry cLight identity, camera/store and optional F8BF4C graph
composition remain separate work. Null F8BF4C or invented IDs are not admitted.

## Verification and limits

`scripts/build.ps1` exited0; MSVC Win32 Release compiled both changed units and
the normal host. Enabled CTests `reconstructed_math` and `tool_tests` passed2/2.
One ignored focused production-Source case linked that rebuilt `bsp_core.lib`
with `/MD /MANIFEST:EMBED` and passed. It constructs the actual raw string pool
and directional pool, real counter-derived type lineage and shared scene/lifetime
runtimes, using tables and constants extracted from installed Original SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The case checks actual borrowed-header name copy, constructor-unwritten tail
sentinels/slab identity, actual +4 reference identity/count1, queued retain2,
logical release leaving1 and the same live bindings/name, then terminal release
returning the raw name exactly once and reusing the SAME physical slot. Final
pool/list/string retirement and normal interruption=false pass. Fixture shadow
resolution throws if unexpectedly requested; constructor174 is genuinely null.
No production shadow provider is fabricated. Evidence is under ignored
`local/cc12_native_gui_raw_directional_lifetime_60/` (build/probe logs, extracted
data, preparation/compile scripts and the one probe source/executable).

No new relocated Original differential is claimed: the constructor kernel and
complete destructor already have separate native evidence. This packet proves
the typed raw Source composition and focused lifetime, not Original outer ABI,
native exception identity, normal Screen startup/drawing or game behavior.

Root primary Source review passed the full four-file diff, existing constructors
and both scene/lifetime pointer-vector admission paths before commit. Actual
header/context/constant identity remains a caller obligation where this narrow
environment has no independent expected owner to compare; preflight borrows
those required cells and does not claim to authenticate them against a new map.
