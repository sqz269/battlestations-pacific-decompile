# Native shader-owner texture unload

The complete `00B188A0` child and `00B24E20` renderer loop now operate on
actual borrowed owner/renderer storage and the existing
`NativeRendererTextureBindingContext`. They compose the full current 2D,
cube and volume texture destruction providers. They do not construct a
renderer, shader owner, registry, replacement reference count or service
callback. The blocked reload/Lua/texture-loader family remains separate.

The implementation is in `include/bsp/native_shader_owner_texture_release.hpp`
and `src/native_shader_owner_texture_release.cpp`. Its reviewed source base is
`3ea154d`; the complete SHA is in the companion audit. Original ABI for both
entries is ECX receiver, no stack arguments, plain `RET`, with no stable
semantic EAX result. The new C++ interfaces pass their context explicitly and
are not drop-in native ABI entries.

## Complete behavior

`unload_native_shader_owner_textures_00b188a0` initially skips only a zero
16-bit texture count at owner `+38h`. At each iteration it sign-extends that
current short to a DWORD and compares the unsigned index. If the index is at
least the bound, it writes the low short of `index+1`. It then captures the
texture pointer at owner `+0Ch + index*4`.

A nonnull texture is decremented with `InterlockedDecrement(texture+4)`.
Exactly zero invokes the captured object's current slot-zero operation.
After that operation returns, the shader slot is set to zero. A nonzero
decrement result also reaches the zero store; a slot already null skips both
the decrement and the store. The next comparison uses a fresh current count.
There is no final count reset, registered-name destruction, error-texture
release, secondary-object cleanup or registry mutation.

The first dispatch reads the current actual texture profile and slot zero.
For the admitted profiles, that word is `00BD30E0`. Its complete behavior
reloads the current object table and calls slot `+4h` with deleting flag 1.
The file-local helper performs those two separate table reads and selects
the established full source operations:

| Current profile | Deleting word | Concrete provider |
| --- | --- | --- |
| `00D61948` | `00B3F590` | `delete_native_texture_2d_00b3f590` |
| `00D61870` | `00B3F410` | `delete_native_cube_texture_00b3f410` |
| `00D618B0` | `00B3F430` | `delete_native_volume_texture_00b3f430` |

The existing context supplies the actual canonical pools, retained-memory
owners, string allocation, renderer publication, synchronization, resource
support and lifetime domains required by those providers. Numeric original
profile/call words select complete source operations; they are not callable
host addresses. Unsupported profiles are outside the declared domain.

`unload_native_renderer_shader_owner_textures_00b24e20` initially reads
renderer count `+1AA0h`, then base `+1A9Ch`, and computes a DWORD end with
stride `2Ch`. It invokes the complete child on current record `+28h`. After
the child returns it reloads count, then base, computes the current end,
advances the **old cursor** by `2Ch`, and tests equality. It does not rebase
the cursor after reentrant registry changes or reinterpret the count as a
signed-positive loop condition. It neither retains nor null-checks record
owners. Renderer profile `00D5F0A8` selects this entry at slot `+124h`.

Neither original body has an EH frame. The source adds no guard or cleanup
and deliberately allows terminal exceptions to propagate before slot zero.
A throwing destructor can therefore leave the shader slot containing the
captured texture address after the texture's own partial/unwind work. There
is no compensating retain, retry or rollback.

All accessed raw storage must remain valid at the native accesses. Negative
short counts become large unsigned bounds, and address arithmetic wraps as
DWORD arithmetic. No bounds repair or mutation-safe container contract is
claimed. The separate count-growth store remains present even though it is
redundant when no intervening state changes occur.

## Verification

The strict MSVC Win32 Release build through `scripts/build.ps1` passed with
`/W4 /WX /fp:strict`; both existing CTests passed. All eight native math seed
spans were freshly matched to the installed executable before that build.
An ignored `CMAKE_PROJECT_INCLUDE_BEFORE` hook registered only this new source
in the worker build. No tracked CMake file or permanent test was changed.

The ignored differential fixture links the frozen, complete `bsp_core.lib`.
It runs the original 84-byte child and complete 65-byte parent at their
original private addresses. **Neither owned body has a patched byte or
relocation.** Their direct parent-to-child edge remains original. Three
external deleting entries have declared five-byte ABI bridges to the full
actual library providers; the InterlockedDecrement IAT cell has its declared
genuine atomic forwarding binding. This compares the original unload control
flow with the source using the same actual destruction providers, rather
than independently revalidating every original texture destructor.

| Focused paired case | Established observation |
| --- | --- |
| Zero count | No texture access, release or shader-slot write. |
| Retained pointer and null slots | Nonzero decrement still clears the retained slot; already-null slots receive no writes; count 3 remains 3. |
| Complete 2D, cube and volume release | Actual retained-memory/name/registry/COM/pool lifetimes complete before shader-slot zero; successful deletion returns the canonical pool slot for reuse. |
| Nested parent traversal | After a real COM Release, both current registry base and count change. A nested parent clears the next shader first; the outer old cursor subsequently visits that null slot and then the third record. |
| Current child count | A real COM Release callback extends the current count from 1 to 3; the child reaches and clears the later retained slot. |
| Throw after real COM Release | Full cube provider unwind runs and the exception propagates through the original/source parent; the captured shader slot remains uncleared. |

All eight pairs match: **19,176 DWORDs across 204 observation frames**, with
34 actual COM calls, 54 observed data writes and six explicit mutation records
across the two arms. All 96 genuine COM objects are cleaned. The fixture uses
hidden HAL devices and forwards real COM AddRef/Release operations before
observations or deliberate mutations. It uses actual canonical pools,
retained-memory/string/surface providers and CRT allocation/free. Additional
real COM references keep receivers alive for observation. Private renderer
and shader headers are initialized valid input storage; their constructors
and game instances are not under test.

The audit additionally verifies 26 fresh live-Ghidra/installed-PE spans totaling
819 bytes; 442 before/after original-image postimages; the entire 70,331-byte
fixture `.text` section unchanged at runtime; 32 exact COFF archive members;
79 source/header closure pins; full compiled ranges and observed instruction
sites for the owned functions and reached providers; and original COM table
entries plus loaded COM/CRT/Win32 module identities and relocated disk prefixes.
The standalone allocator observation object is the complete current
`singleton_lifetime.cpp` compiled with only its allocation/free entry names
changed, then reached through wrappers that forward its actual CRT behavior.
It is not an injected allocation-result or owner-destruction substitute.

MSVC inlines the complete child into the parent and the cube destructor into
its deleting entry. The pinned full source, COFF members and compiled ranges
cover those operations; the audit does not require a separate linked symbol
for every inlined body. Compiled inspection confirms the unsigned comparisons,
MOVSX/MOVZX count reads, short growth store, delayed DWORD zero stores and
old-cursor/current-end arithmetic. Malformed negative-count traversal and a
concurrent change specifically between the continuation read and growth read
were not executed as differential cases.

Every live capture used the guarded `tools/bsp.py ghidra` command and verified
project `bsp`, program `/battlestationspacific.exe`, x86 language and image
base before dispatch. The configured analysis project remained
`C:/Users/sqz269/bsp.gpr`. No Ghidra, installed game, shared ledger or shared
build registration was changed by this packet. Build and fixture evidence
does not establish a drop-in ABI replacement, device-reset scheduler,
whole-image identity, shader reload or game validation.

## Primary integration

The primary verified all 211 immutable worker hashes and reviewed all 79
current source/header files: 77 were identical, one header had comment-only
changes, and one added an unrelated declaration without changing existing
layout or declarations. Twenty-six fresh live-Ghidra/PE spans (819 bytes),
the strict main build, both existing CTests and eight fresh seeds passed.

The unchanged fixture linked only the actual main library
`0a69eb17fe45665164dc246dfde4fa71a675a6d8f951b9bb7722c7a6b766d4a8`.
All 32 linked archive objects were frozen and matched the actual main objects;
all 79 source/header files were snapshotted. Eight paired cases match 19,176
normalized DWORDs and 204 frames, including 34 actual COM calls, 54 observed
writes and six explicit mutations. All 96 genuine COM objects were drained.
Both native owned bodies remain pristine, and all 442 original-image
postimages and the whole 70,331-byte fixture text are verified. The two
complete owned COFF sections match linked code with exact symbol relocations.

The child and parent received reviewed descriptive names and appended
evidence in Ghidra, preserving old names/comments in the saved journal.
Full reconstruction records and forced exports are registered. The fixture
shares the complete current source destruction providers through the declared
external bridges; it does not repeat original texture-destructor validation.
Reload/Lua/resource-loading dependencies, complete reset scheduling, original
caller ABI and gameplay remain outside these two verified routines.
