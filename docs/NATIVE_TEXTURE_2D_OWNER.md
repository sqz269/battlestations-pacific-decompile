# Native texture2D construction and destruction

The new interfaces in `include/bsp/native_texture_2d_owner.hpp` implement the
complete three texture2D owner entries over the application's actual storage.
The evidence record is `reports/native_texture_2d_owner_audit.json`.

| Native entry | Complete range | Native ABI | New C++ interface |
| --- | --- | --- | --- |
| `00B3F930` | `[00B3F930,00B3FA81)` / 337 bytes | ECX owner; stack name, input COM, saved width, saved height, flags; EAX owner; RET 14h | `construct_native_texture_2d_00b3f930` |
| `00B3F2E0` | `[00B3F2E0,00B3F40E)` / 302 bytes | ECX owner; plain RET | `destroy_native_texture_2d_00b3f2e0` |
| `00B3F590` | `[00B3F590,00B3F5B0)` / 32 bytes | ECX owner; stack flags; EAX original address; RET 4 | `delete_native_texture_2d_00b3f590` |

These are new MSVC Win32 C++ interfaces. Their original calling conventions are
documented above; a binary replacement ABI and game execution are not established.
The raw owner occupies 50h bytes of the canonical pool's 54h-byte slot. All three
entries preserve the pool's trailing slab index at +50h.

## Actual dependencies and supported profiles

`NativeTexture2DOwnerContext` borrows the existing renderer notification context,
retained-memory context, surface context, canonical `D3D9Texture2DPool`, shared
texture serial, and texture tracking counter. Notification and surface contexts
must reference the same renderer publication and string pool. The surface context
also supplies the actual resource-support publication and lifetime domain.
No second intrusive count, string ownership system, COM owner, renderer list,
singleton manager, or slot-return policy is introduced.

Current native table words select the already reconstructed implementations:

- renderer `D5F0A8`, slot +6C = `B32250`;
- surface `D619A0`, slot 0 = `BD30E0`, slot +4 = `B3F5B0`;
- memory backing `D15AD8`, slot 0 = `BD30E0`, slot +4 = `8D4470`;
- memory stream `D642C0`, slot 0 = `BD30E0`, slot +4 = `BB8F90`.

The selected tables are borrowed actual immutable storage. Unsupported profiles
are outside the interface's domain. COM entries are called through each actual
current interface table, with the original stdcall argument shape.

## Construction order

The full named profile constructor `B34230` establishes the named base, count1,
borrowed COM, input flags, and shared serial increment. `B3F930` then installs
`D61948`, initializes its observed fields and cache header, and clears +4C.
It captures the current owner COM before `GetLevelCount`, stores that result,
and restores the input flags. The following AddRef/Release pairs and
`GetLevelDesc(0)` use the captured input COM, even if the first COM callback
changes the owner field. Neither pair transfers ownership.

The HRESULT is ignored. Width and height are captured from the 32-byte descriptor
before the second pair; format is read afterward. Saved width/height and the
remaining owner fields retain the assembly's store order. No descriptor zero
initialization or failed-HRESULT fallback is added.

The stack diagnostic record borrows the input COM and copies its name through
the actual string domain. The direct `BF7680` copy uses overlap-preserving
`memmove`. The copied buffer is captured separately from the current diagnostic
header. After the actual support-singleton getter returns, normal cleanup frees
that captured buffer with the current diagnostic length+1. Exceptional state2
cleanup instead runs the existing `B3F4C0` action on the current diagnostic
record. Only input flags bit10h controls the constructor's tracking increment.

Construction does not allocate the owner slot, retain a source, register the
texture, or add an owned COM reference. Failure leaves those caller obligations
with the caller; no owner-slot return or COM-release rollback is synthesized.

## Destruction and scalar deletion

The destructor installs `D61948` and captures +4C. It decrements the actual
retained source count and, only at zero, follows its current virtual-zero and
current deleting slot with flag1. The owner slot is cleared after that terminal
returns. Notification captures the global renderer and its table-slot address,
obtains the owner's original name address, and then reads the captured slot.
Texture unregistration uses a fresh global renderer read after notification.

The current COM receives a captured balanced pair, support is accessed, and a
fresh owner COM is released and cleared after return. Each cache iteration reads
the current array and captures that iteration's surface-pointer slot. A nonnull
surface receives a real intrusive decrement and current deleting dispatch only
at zero. After return, the captured slot is cleared, even if its callback replaced
the array. The next signed loop-bound check and array read are current reads.

Current flags bit10h controls the tracking decrement. Cache storage is destroyed
before the named base. The full post-free tail at `B3F3E9..B3F40D` is included;
the saved Ghidra function body originally stopped at `B3F3E8` because of the
suspect free no-return boundary. The integrator owns that analysis repair.
Scalar deletion returns the canonical pool slot only after successful complete
destruction and flags bit0. Its result remains the original, possibly reusable,
address. An exception does not retry source, COM or cached-surface release.

## Cleanup-only exception states

Original constructor FuncInfo `DF783C` has three unwind states, map `DF7824`,
and zero try blocks: state2 diagnostic `CBEF63`, state1 cache `CBEF58`, state0
base `CBEF50`. Destructor FuncInfo `DF7710` has two states, map `DF7700`, and
zero try blocks: state1 cache `CBEE98`, state0 base `CBEE90`.

The C++ entries use a cleanup-only RAII action with explicit native state.
Cleanup catches no C++ exception. Its SEH search filter terminates for a second
C++ exception before nested unwinding, matching the original FH3 behavior.
Normal cleanup disarms the relevant state before entering each corresponding
native action. Final MSVC listings confirm `nTryBlocks=0` for both owner entries.

## Validation and reproducibility

The preparation command `python local/prepare_native_texture_2d_owner.py` freshly
verified 46 pinned code, EH, profile, bridge-prefix and global/IAT spans against
the installed PE through the guarded Ghidra CLI. Every query verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, language and image base.
No game file or Ghidra state was modified by this packet.

One ignored fixture, `local/native_texture_2d_owner_check.cpp`, executes complete
original owner3 bodies, their original FH3 handlers/maps/funclets, original
cache3, diagnostic cleanup, name-address leaf, virtual-zero dispatch, and texture
unregistration. Explicit ABI bridges invoke the unchanged current named-base,
notification, retained-memory, surface, canonical-pool, support and string APIs;
CRT allocation/free/copy and the host FH3 runtime are declared boundaries.
It uses actual shared domains and actual Win32 critical-section operations.
The COM endpoints are deterministic ABI fixtures, not a D3D device or GPU.

The fixture reserves its native address range in its own suspended child before
loader heap initialization, then resumes that child and commits only pinned
pages. Original profile words and virtual table entries consequently need no
numeric relocation or field normalization. Gaps remain inaccessible or CC-filled.
Code/EH/tables become execute/read. Two verified registration operands point to
host EXE jump-only trampolines that enter the complete original FH3 handlers.
All other owner-body bytes remain original. The game executable is read-only.

`./local/build_native_texture_2d_owner_check.ps1` strictly compiles and links the
actual owner and dependencies, and compares complete 67-DWORD observation frames.
Pointer observations use allocation identity plus offset; already freed
diagnostic identities remain tied to their original allocations despite later
heap address reuse. Returned surface slots are inspected as still-allocated slab
bytes, without calling methods on an ended C++ object lifetime.

Five normal/throwing states match **8,978 DWORDs / 134 events**: odd successful
deletion with COM/descriptor/diagnostic/cache mutations; even deletion through a
direct backing and both original cache surfaces; constructor COM failure;
state2 support-allocation failure with a replaced diagnostic header; and
notification failure after real critical-section entry. That last case retains
the actual OS recursion1, tracked depth0 and nesting1 while owner cache/base
cleanup completes, with no COM/surface retry or owner-slot return.

Two additional native/host process pairs match **737 DWORDs / 11 events** and
independently exit86: a second allocation exception during cache cleanup with
current negative capacity, and a constructor body exception during an outer
unwind. Both terminate during search with uncaught count1, nested probe cleanup0,
`D61948` still installed and the base name still allocated.

`./local/build_native_texture_2d_owner.ps1` runs `./scripts/build.ps1` with an
ignored deferred CMake include for the owner and current cache dependency.
The strict Win32 build and both existing tests pass; all eight native math seeds
were verified before the build. No tracked test or shared CMake/ledger change
belongs to this packet. Reproducers, logs, compiler listings, binaries and current
source hashes are pinned in the audit. Shared registration, Ghidra annotation,
body repair, ledger entries and refreshed exports were completed by primary
integration after the worker evidence was reviewed.

The primary build now registers this source in `bsp_core`. Its three owner
symbols and selected dependencies resolve to a frozen copy of the current
library; exact current allocator/string/synchronization sources are compiled
separately only for the existing fixture observations. The same 9,715 DWORDs
match, including both terminal pairs. All 46 fresh Ghidra/PE ranges and every
loaded postimage were checked. Ghidra now includes the destructor's returning
free continuation through `B3F40D`; prior comments and earlier high-level
fragment records were preserved. Saved annotations and refreshed exports are
recorded in the audit. Both existing CTests pass; no tracked tests were added.

Unrestricted COM implementations, other native profiles, invalid accessed
storage, unobserved fault sites, concurrent mutation, whole original dependency
closure, game behavior, and visual/GPU correctness remain outside this proof.
