# Native texture device reset

This packet owns full `B3DD30` (86 bytes), `B3DD90` (362 bytes), `B33F10`
(1 byte), and `B33F20` (3 bytes), with the separate 16-byte pool table at
`B3DEFC` as read-only evidence. These are the actual 2D, cube and volume reset
callbacks reached through texture profile `+20/+24`; they do not implement
the renderer's registry walk or full device recreation.

## Frozen source contract

The source borrows the actual texture owner and current raw level records.
Every reached nested wrapper is actual `NativeSurfaceOwnerStorage` with
current `D619A0` profile. The context borrows its actual profile words through
`+3F`. Source reads the current owner profile and relevant slot at the native
accesses, then selects the existing complete `B3CC80` bind or `B3D510` release
provider. Numeric original method addresses are evidence tokens, never host
function pointers. Unsupported profiles are outside the API domain.

Restore takes two explicit mutable four-byte output addresses. The caller
seeds the device-argument/output cell with the actual incoming device bits.
At entry restore seeds the reused surface-output cell with the owner bits,
matching `PUSH ECX`. Native entry ESP `S` puts these cells at `S+4` and `S-4`;
the new API exposes both addresses without claiming a host stack layout.
Callers that need the native relative positions can supply those positions.
Neither cell aliases actual owner, record or profile storage.

The device cell is both the incoming device argument and `CreateTexture`'s
output. Restore captures the receiver before passing the cell. An unsupported
pool nibble also copies the incoming device bits. After creation, it reads
current owner `+10`, then the current output cell; when distinct it publishes
the new pointer, AddRefs captured new if nonnull, and releases captured old
if nonnull. It reloads the output after each actual callback. A nonnull current
temporary receives Release, then its argument cell is cleared after return.
No HRESULT branch or rollback changes this sequence.

The reused surface cell initially contains the owner pointer, and survives
all levels without per-level initialization or clearing. Each iteration
captures current texture/table, then current record base and level key.
After `GetSurfaceLevel`, reload the record base and nested destination owner.
Read that owner's profile, then the output cell, then its `+14` slot. Call
the actual bind provider, reread the output, Release it unconditionally, and
then compare incremented index with the freshly loaded signed count.

Release visits current records in signed index order, reloading base before
each wrapper and count after its current `+3C` callback. It then captures
owner `+10` for an AddRef/Release pair and reloads `+10` for its final Release
and clear. Exceptions propagate at the throwing call; these entries have no
cleanup scope or added retry.

The one-byte `RET` and three-byte `RET4` callbacks are evidenced complete
no-ops for the cube/volume profiles, not substitutes for unknown behavior.

## Validation

`scripts/build.ps1` passed under MSVC Win32 Release with `/W4 /WX /fp:strict`,
including both existing reconstructed-math and native-math checks. A private
deferred CMake registration added this source to the actual `bsp_core` build;
no tracked CMake or shared metadata file was changed. The primary library,
texture-reset object, surface-owner object and their source/header hashes
were frozen before the private comparison was compiled. The fixture compiles
only its adapter/observer file and links that immutable primary library.

The original and source runs produced identical **17,134 DWORDs across 183
snapshots in 17 focused phases**, hash
`3f70ba1fa15788ac293bf3d2701fde3636680c2125e2c686882b0cb08b90cd21`.
Each process used one real HAL device and executed 13 `CreateTexture`, four
`GetSurfaceLevel`, four `GetDesc`, and 30 AddRef/Release calls. Six creates
succeeded; seven returned actual `D3DERR_INVALIDCALL` and wrote null output,
including a case with a nonnull old owner that was still released. Creation
arguments cover pool codes, usage groups, invalid pool/device bits, and wrapped
dimensions, level count and format.

The mutation cases force count growth from one to two, change the record base
between key lookup and destination binding, replace the current next-level
texture, and replace both explicit output cells at the relevant COM callbacks.
They distinguish captured reference pairs from fresh owner loads, fresh
descriptor receivers from captured input surfaces, and the final output clear
from a callback's intervening write. The identity path, negative count, seeded
and reused scratch, two throwing callbacks, and genuine no-ops are covered.

All 452 owned bytes, 152 bytes of the two original surface providers and the
16-byte pool table are checked against fresh guarded Ghidra bytes and the
installed executable. Original runtime code differs only by the pool-table
absolute operand and its four target DWORDs. The two reached surface profile
cells and native wrapper table pointers are relocated into borrowed data;
source wrappers retain the original token and use the explicit context. No
original instruction is replaced with a helper or service callback.

The audit verifies every observed caller return address against a CALL in the
original bodies or the six functions linked from the frozen library. It also
verifies the saved interface-table method against the actual x86 D3D9 module,
the factory import thunk/IAT, all original code/table postimages, and the
entire runtime executable text against its relocated PE. Whole-owner, wrapper,
record and output snapshots capture native effects; all 23 fixture field/output
writes are listed separately. This is not a hardware trace of each store.

Observers restore and forward real Windows COM methods before applying their
explicit mutations or throw markers. Extra fixture references keep all reached
replacement objects alive; their ownership is separate from production calls.
All 181 actual fixture COM objects were released to zero during teardown.

These are new C++ APIs, not original ABI replacements. The fixture does not
exercise a full renderer/device reset, draw or game. Successful level/descriptor
outputs are covered; their HRESULT failures or invalid unwritten pointers are
not repaired or claimed tested. Failed-create output behavior describes the
observed driver and does not define a general driver policy.

## Integration

Only the header, source, this document and audit report belong to this packet.
The integrator registers the source, applies evidence comments and ledgers,
and refreshes exports. `B33F10` and `B33F20` currently lack saved function
definitions; define their exact one-byte and three-byte bodies before naming
them. Existing `B3DD30/B3DD90` names and comments should be preserved.

## Primary integration

The primary registered the source in CMake and passed the strict Win32 build
and both existing checks. It verified 52 worker artifact pins, four current
source/provider files and 12 fresh live-Ghidra/installed-PE spans (652 bytes).
The unchanged fixture linked frozen actual main library `e4722daa6979b84a72afd085312ad7f5b9670e601763a21b1262720b938c923d`.
Both real-HAL runs again matched 17,134 DWORDs, 183 snapshots and 17 phases,
including seven actual INVALIDCALL/null CreateTexture results per process.
All 51 call origins per process, real COM slots/modules, factory thunk/IAT,
whole executed source text, complete native code and tables were checked.
The 23 labeled mutations are fixture writes; no hardware store trace is claimed.
The exact one-byte B33F10 and three-byte B33F20 functions were defined and
saved before all four names received appended evidence. Full records and
forced export refreshes are registered. No permanent tests were added.
