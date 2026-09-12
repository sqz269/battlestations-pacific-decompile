# Actual PointLight ownership

Addresses: `00B7C710`, `00B7C770`, `00B7C850`, `00B7C740`, `00CD81A0`,
`00B7B690`, `00B7B770`, `00B7B810`, `00B7B1D0`, `00B7B110`, `00B7ABD0`,
`00B7BD30`, `00B7B600`, `00CD8060`, `00CE0EA0`; shared `00B7C4C0`, `00B7C5B0`.
Descriptive names are hypotheses; the C++ interfaces are new, not binary ABI replacements.

PointLight now has an owning C++ composition using its actual **200h** pool
slot, original self-reference at `+04`, canonical scene/node binding and physical
backlink array at `+1E0`. The point pool is native `0109011C`, with slab ID at
`+1FC`. It cannot share the directional pool's 1F0h geometry or `+1EC` slab ID.
The native PointLight constructor leaves `+1EC..+1FB` unwritten. Position/radius
writers and render-value projection into those words remain separate work.

| Routine | Native ABI | Coverage |
| --- | --- | --- |
| B7C710 | ECX slot; name on stack; EAX same slot; RET4 at B7C73A | Complete constructor |
| B7C770 | ECX light; RET at B7C7DF | Complete valid-storage destructor and reused Light cleanup |
| B7C850 | ECX light; deletion flags on stack; EAX original slot; RET4 at B7C86D | Complete scalar deleting wrapper |
| B7C740 | Token on stack; AL bool; RET4 at B7C760/B7C765 | Complete current four-token predicate |
| CD81A0 | No arguments; RET at CD8240 | Complete guarded type bootstrap |
| B7ABD0 | ECX slab; index on stack; EAX slab; RET4 at B7AC10 | Complete slab producer |
| B7B690 | ECX pool; EAX pool; RET at B7B762 | Complete valid-storage pool construction and cleanup |
| B7B810 | ECX pool; EAX slot; RET at B7B92B/B7B93F | Complete valid-storage allocation |
| B7B1D0 | ECX pool; raw slot on stack; RET4 at B7B225 | Complete pool return |
| B7B770 | ECX pool; RET at B7B809/B7B80F | Complete trim including installed-byte continuation |
| B7B110 | ECX pool; RET at B7B199 | Complete pool destruction including installed-byte continuation |
| B7BD30 | No arguments; tail JMP B7B810 | Complete allocation thunk; old static-dtor label is wrong |
| B7B600 | Slot in ECX; RET at B7B60B | Complete static pool-return thunk |
| CD8060 | No arguments; EAX atexit result; RET at CD8075 | Complete installed-byte static-init span; parent defined matching-byte Ghidra body |
| CE0EA0 | No arguments; tail JMP B7B110 | Complete atexit pool-destruction thunk |
| B7C4C0/B7C5B0 | ECX light; constructor stack name RET4; destructor RET | Existing complete implementation reused; reference-only destructor adapter added |

The type owner borrows the same `TypeIdCounterLifetime`, root/node/light
descriptors and guards used by `LightTypeBootstrap`. CD81AD captures the light
guard before publishing the point guard and native name `00D62F30`. Parent
initialization follows that captured decision; all three inherited tokens are
captured before stores. The current predicate compares four live words without
running an initializer. No default or privately allocated type IDs exist.

## Storage and lifetime

The existing Light constructor establishes the native Node prefix and base
words. Its temporary `NativeLightTailStorage` lifetime is ended before the
distinct `NativePointLightTailStorage` starts. The latter has the exact same
base fields through `+1DC`, followed by the physical descriptor. No direction
reference overlaps that descriptor. Constructor-unwritten specular, mode3,
scale and `+1EC..+1FB` words preserve their allocation preimage. The pool's
`+1FC` ID is untouched by construction and destruction.

The stable companion contains `NativeNodeBinding`, `LightSceneRetention`
referencing actual `+178`, and `NativePointLightLinksBinding` referencing actual
`+1E0`. The owning factory binds all three to the existing scene, physical-link
and lifetime runtimes. The native node identity, scene-registry key, queued
reference and physical light-pointer arrays all denote the same raw allocation.
No additional scene array, backlink list, root map or reference count is stored.
The string service remains the existing explicit `SizedStoragePool` adapter,
with the same boundary as the reused Light/Node constructor.

B7C770 publishes the point phase, invokes the existing complete B7C160 unlink
pass, repeats resize0 and frees its reverse backing. Both duplicate forward
entries are removed when two reverse entries name the same node. The descriptor
is unbound before the Light base phase and before its typed lifetime ends.
Freed pointer/capacity words remain unchanged. The existing backing-domain
provenance checks reject foreign arrays; this owner does not adopt foreign memory.

The shared Light destructor now accepts a reference-only adapter over shadow,
retained scenes, node and a tail-lifetime-ending callback. The existing
DirectionalLightOwner overload constructs that adapter, preserving its previous
behavior. PointLight uses the same scene-drain callback reloads, actual shadow
reference release, array cleanup, and complete direct Node destructor. Node
phase becomes `00D62C88`; subsequent reference-base teardown leaves `00CEB130`
in destroyed storage. The point companion records dead only after this cleanup.

The point EH descriptor `00DFAFF8`, map `00DFAFE8`, maps state1 to reverse-array
cleanup at `00CC1EF8 -> 005A1610` and state0 to Light destruction at
`00CC1EF0 -> 00B7C5B0`. Under the supported valid-descriptor/owned-backing
contract, unlink/resize0/free invokes no throwing external callback. Light-phase
exceptions run the existing Light cleanup and do not return the pool slot.
The adapter diagnostics are not native malformed-memory or SEH equivalence.

`NativePointLightReference` borrows the actual atomic `+04`. Current profile
checks cover native 00,04,0C,18,34,40,50,54 in the caller's immutable22-word
tables. Shared B6F310 logical release can leave the same owner alive for queued
references; BD30E0 terminal dispatch supplies flags1 to B7C850. Final deletion
unbinds the existing lifetime association and retires both host companions.
Once a reference companion is bound, callers must use that reference lifetime;
direct owner destruction is for an owner without a live reference companion.
Direct B7C850 flags0 ends native lifetimes but leaves the same backing allocated
for an explicit later return to the original pool. The factory's extra host
setup-failure rollback applies only to unpublished fresh construction.

## Pool evidence and boundaries

B7ABD0 writes32 reverse free indices at slab`+4000`, free count32 at `+4040`,
and index words at each slot`+1FC`, leaving other slot bytes and slab`+4042`
untouched. Allocation uses real CRT storage for4044h slabs and the mutable
pointer table, real Win32 critical sections, and the shared allocator list.
B7B1FC uses signed `SAR ECX,9` for slot return. Trim moves the last slab into
each erased position and rewrites all32 moved slab IDs before retrying that
index. Allocation failures preserve the native publication/lock behavior;
there is no invented pool rollback or automatic unlock.

Current Ghidra omissions are recorded as explicit raw-body repair requests:
CD8060..CD8075 has no defined function; B7B770 is missing the post-free58-byte
span B7B796..B7B7CF; Light B7C5B0 ends at its falsely no-return free. Installed
bytes prove B7C686..B7C6A9, including B7C693 -> B6F440. B7B110 also has the
post-free loop increment/cleanup gaps B7B12D..B7B137 and B7B145..B7B147. Small
post-free cleanup gaps in B7B690/B7B810 are likewise documented. No missing
body call is attributed to a different containing function. No Ghidra writes
were made by this worker; annotation/definition repair belongs to integration.

The untouched point vtable getters at B7AB40/B7AB50, arbitrary virtual slots,
render-value producers, scene-provider creation, nonempty shadow construction,
pool concurrency/recovery beyond native behavior, and original-binary exception
execution remain outside this packet. Named existing dependencies still require
their own actual configured services; their names alone are not completion proof.
The rebuilt game does not yet configure/reach this new PointLight owning factory.

## Verification

The new owner/pool and modified shared Light source compiled with MSVC Win32
`/std:c++17 /W4 /WX /O2 /MD /EHsc /fp:strict`. One local probe linked those
objects to the existing current `bsp_core.lib` and real CRT/Win32 services with
`/MANIFEST:EMBED`. It passed guarded type ancestry,33-slot allocation and moved
slab-ID repair, actual200h construction, preserved unwritten bytes, raw identity,
flags0 retained backing, actual shadow/node reference-zero callbacks in the
proper base phases, duplicate physical links and queued terminal flags1.
An initial fixture expectation incorrectly used the Node phase as the final
profile; existing B6F552 -> BD30F0 evidence corrected it to `00CEB130`.

The report records exact call rows, source/probe hashes and verification results.
The integrator must register `src/native_point_light_pool.cpp` and
`src/native_point_light_owner.cpp` in CMake and run `scripts/build.ps1` after
merging. No full worker build tree or permanent test suite was added. These are
source/fixture checks, not native-byte differential, game, render or binary ABI
validation.

## Parent integration correction

The parent defined the matching CD8060..CD8075 body, repaired the Point pool's
local false-free fall-through gaps, and extended the B7C5B0 destructor through
B7C6A9. B7C693 now belongs to that body and calls B6F440. The three previously
raw-only call rows are promoted into the final mechanical call check, with their
original observations retained in the report. Global CRT no-return annotations
were preserved. The combined Win32 build, both existing tests and a fresh probe
linked to the final combined library passed. See
`docs/ORCH5_SECTION_FRAME_LIGHT_BATCH.md` for reviewed integration limits.
