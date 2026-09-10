# Native directional light ownership

This packet reconstructs the base-light constructor `00B7C4C0`, directional
constructor `00B7C6B0`, complete light destructor `00B7C5B0` and directional
deleting destructor `00B7C820`. They operate on the actual 496-byte slot from
the existing `DirectionalLightPool`. Native constructor ABI is ECX owner,
stack name, EAX same owner, `RET4`; the body destructor takes ECX and returns
with `RET`; the deleting destructor takes stack flags and returns the original
raw address with `RET4`.

The implemented profile includes the native constructor's proven null shadow.
It also releases a supplied nonnull shadow through its real identity/count and
final-release binding. Creating the concrete shadow owner, cameras and texture
resources remains a separate packet. No placeholder shadow or default runtime
type ID is installed by this code.

## Actual storage and borrowed consumers

The existing `NativeNodeStorage` occupies `+000..+173`. A separate exact
`NativeLightTailStorage` occupies `+174..+1EB`. Neither typed object overlaps
the live pool slab ID at `+1EC`. `NativeLightStorageView` contains references
to those two objects and has no copied native state.

The tail contains the actual shadow identity at `+174`, the actual
`SystemAmbientBacklinks` pointer/count/capacity at `+178/+17C/+180`, and these
mutable raw-word fields used by the shared lighting consumers:

| Offset | Field |
| --- | --- |
| `184` | `diffuse_184` |
| `194` | `specular_194` |
| `1A4` | `base_diffuse_1a4` |
| `1B4` | `diffuse_mode3_1b4` |
| `1C4` | `base_specular_1c4` |
| `1D4` | `scalar_1d4` |
| `1D8` | `diffuse_scale_1d8` |
| `1DC` | `specular_scale_1dc` |
| `1E0` | `direction_1e0` |

The stable external `DirectionalLightOwner` companion contains the existing
`NativeNodeBinding`, a `LightSceneRetention` referencing that same actual
`+178` array, and a `SystemDirectionalLight` referencing the same color and
direction words. The scene attachment's directional-light projection points
to that view. There is no additional light registry or retained-scene array.

The shared raw-shadow accessor reads the actual `void*` slot at `+174` every
time. Null returns null without invoking a resolver. A nonnull identity must
resolve through `SystemShadowOwnerResolver` to its real borrowed
`SystemShadowMapOwner`; it cannot silently produce null or a cached owner.
Ownership is separately bound through the same `NativeNodeDestructionRuntime`
used for node `+130`: actual identity, actual atomic count at identity `+04`,
and required actual virtual-00 final release. These interfaces do not create
the shadow or substitute a host vtable for raw native bytes.

Raw construction and companion binding are separate from allocation. The caller
allocates the slot from its canonical pool, calls the directional constructor,
creates a stable companion with actual directional/light predicates and shadow
resolver, then binds its existing scene attachment before exposing it. The
companion's ordinary C++ destructor has no native ownership side effects.

## Constructor and vtable phases

`00B7C4C0` first runs the existing node constructor. It publishes light vtable
`00D62F58`, clears shadow and the retained-scene array, writes four raw
`3F800000` words at both `+184` and `+1A4`, writes `(0,0,0,3F800000)` at
`+1C4`, writes actual constant `42800000` from `00CE7820` at `+1D4`, and writes
`3F800000` at `+1D8`.

The native constructor does not define `+194..+1A3`, `+1B4..+1C3` or
`+1DC..+1EB`. The C++ implementation reads and restores their actual allocation
bytes when starting typed word lifetimes, preserving those values without
invented colors, scales or direction. The slot's `+1EC` word is untouched.
`00B7C6B0` adds only vtable `00D62FB0` after the light constructor.

The required phase predicates represent actual `00B7C6D0` for directional,
`00B7C580` for light and the supplied node-runtime `00B6F570` for node. Their
runtime type initialization belongs to its own packet. Each phase keeps actual
current virtual-40/50/54 dispatch in the same `SceneNodeAttachment`; children
retain their own dispatch. Node phase uses the existing direct destructor and
its live hierarchy semantics, with no prior virtual-18 release requirement.

## Complete destruction and callback reloads

The deleting destructor publishes directional vtable, then calls the light
body, which publishes light vtable and corresponding dispatch. While the
actual retained-scene count is positive, it unregisters the current back scene
through `00B83EC0`. It then reloads **both array pointer and count** and reads
the new back entry for reference release. It does not release a cached copy of
the first scene. After that scene's zero callback returns, it reloads count
again, decrements when nonzero, and repeats only while the new count is positive.
Valid callback edits must leave a valid entry for the native unconditional back
read immediately after registry removal.

Only after that loop does the body load current shadow `+174`, release its real
reference and call actual virtual-00 on zero, then clear `+174` after the callback.
A replacement shadow installed by that callback is cleared without an extra
release, as in the native body. The body subsequently shrinks the current
`+178` array to zero and frees its actual allocation, leaving pointer/capacity
words unchanged. Scenes newly installed there by the shadow callback are not
drained by an extra loop.

Ghidra's existing body stops at the incorrectly no-return `free` call at
`00B7C681`. Live bytes and installed raw disassembly establish the complete
range **`[00B7C5B0,00B7C6AA)`**: after stack cleanup, `00B7C693` calls the
direct node destructor `00B6F440`, then restores SEH and returns at `00B7C6A9`.
The implementation includes that tail, ending both raw lifetimes and forgetting
only the dead host scene association. Callback-installed native scene state is
not detached or normalized by that association removal.

The deleting wrapper returns the actual slot to the same supplied pool through
`00B7B2F0` only when `flags & 1`. With a clear low bit the destroyed storage
remains allocated. In both cases the return value is the original raw address;
the caller must discard the external companion after destruction.

The `00CC1EE6` handler uses descriptor `00DFAFC4` and map `00DFAFB4`. State-1
unwind performs retained-array cleanup `00B7C1C0`, then state-0 node destruction
`00B6F440`. It does not add a shadow release or scene-drain loop. An exception
does not reach the deleting wrapper's pool-return branch. A second exception
during cleanup terminates in the reconstructed C++ interface.

## Verification boundary

The focused MSVC Win32 C++20 fixture passed `/W4 /WX /EHsc /fp:strict /MD`.
Twenty-four live Ghidra spans totaling 2,738 bytes matched the installed PE.
The reference bundle contains fourteen original function bodies. Thirty
32-bit operands relocate their calls, constants, actual fixture pool and real
CRT/Win32 import boundaries; vtable words are not rewritten.

The native directional constructor and deleting destructor executed for both
flags 0 and 1, including original pool return with actual Win32 critical
sections. All 496 bytes matched before and after destruction; constructor
preimages, all node holes, slab ID, free counts and pool recursion also matched.
The native reference used empty names and no hierarchy, scene, shadow or
point-array ownership. Native nonempty ownership and exception paths were not
executed by that comparison.

The same fixture separately checked that rendering/retention views alias actual
raw fields, null shadow access bypasses resolution, and an unbound nonnull
shadow fails explicitly. Host callback checks changed the array allocation and
back entry inside registry removal, rewrote count during scene zero, replaced
shadow and appended a scene during shadow zero, and observed node phase during
node `+130` zero. Real retained-owner heap allocations were released. A single
scene-zero exception also reached array/node cleanup without releasing the
unreached shadow or returning the pool slot. These checks passed.

The audit records shared-source and local fixture hashes. No tracked test suite
was added. CMake registration and final `scripts/build.ps1` remain primary
integration gates. The new C++ interfaces are not binary replacements, and no
gameplay or visual validation is claimed.
