# Native frame-target group owner

This module reconstructs complete `00B1FBB0`, `00B1FC00`, `00B1FCF0`,
`00B1FAB0`, `00B1FB00`, and `00B1F700` using actual 40h Win32 group storage.
Names are descriptive hypotheses. The callable C++ interfaces are new and
are not drop-in original ABI exports or gameplay validation.

The group has the two-entry native profile `00D5E600`: slot zero
`00BD30E0`, slot four `00B1FCF0`. Adjacent `00D5E608` belongs to the separate
constructor `00B21400`. Discovery and complete original spans are documented
in `docs/NATIVE_FRAME_TARGET_OWNER_DISCOVERY.md`.

## Actual storage and dependencies

`NativeFrameTargetOwnerStorage` is exactly 40h bytes: profile/count at
00/04, four intrusive color surface pointers at 08..14, depth at 18, the
12-byte `NativeFrameTargetVectorStorage` at 1C, four separately retained
COM pointers at 28..34, depth COM at 38, and an exact byte at 3C. Constructor
stores follow the native sequence, including the initial base-profile store.
It leaves padding bytes 3D..3F untouched. There is no auxiliary reference
count, owning C++ container, or host virtual table.

The context borrows the application's existing `NativeSurfaceOwnerContext`
and immutable original `D619A0` table. On a final-zero surface release, the
code loads the current object profile, resolves current slot zero as
`BD30E0`, then reloads the current profile and slot four as `B3F5B0`. It
calls the complete existing concrete surface deleting destructor with flag
one, including actual unregister/support/name/COM/pool behavior. Foreign
surface profiles are outside this concrete precondition.

Vector resize/destruction use the complete `native_frame_target_vector`
provider. Group/vector frees use the existing actual shared CRT service.
The group is not allocated or freed through the surface pool. No allocator,
terminal, or exception callback is invented by this module.

## Original ABI and ordering

| Address | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| `B1FBB0` | ECX storage, EAX storage, RET | Exact ordered initialization; count one; three padding bytes untouched |
| `B1FC00` | ECX owner, RET | Full interleaved surface/COM release, vector disposal, base restoration, EH cleanup |
| `B1FCF0` | ECX owner, stack flags, EAX original address, RET4 | Destroy first; free only after normal return when flags bit zero is set |
| `B1FAB0` | ECX group, stack slot/pointer, RET8 | Unchecked wrapping 32-bit slot indexing; identity skip; publish then retain then release old |
| `B1FB00` | ECX group, stack pointer, RET4 | Same assignment ordering for depth at 18 |
| `B1F700` | ECX group, stack low byte, RET4 | Store the exact byte at 3C |

Destruction processes each color's intrusive field, then its current COM
field, before advancing. An intrusive terminal may change the COM field or
subsequent color fields; later reads observe those changes. The captured
intrusive pointer's +04 count is decremented once. Each field clears only
after its terminal or COM Release returns. Depth follows all four colors.
Assignment has no post-terminal clear or rollback: a callback that changes
the newly published field retains that result even if it throws.

The original `B1FC00` stored Ghidra body omitted the tail following free at
`B1FCC5`. The full 239-byte range includes `B1FCCA..B1FCEE`, its `BD30F0`
base restoration at `B1FCD7`, and the SEH epilogue. Scalar `B1FCF0` is 30
bytes and returns the original address after its conditional free; a
pseudocode `extraout_EAX` is not the return contract. Required Ghidra flow
repairs are reported to the primary; this worker does not mutate Ghidra.

## Exception states

FuncInfo `DF5214`, unwind map `DF5204`, and handler `CBCCE3` establish:

| State | Cleanup |
| --- | --- |
| 1 | `CBCCD8`: complete `B1FB90(this+1C)`, followed by `CBCCD0`: base `BD30F0` |
| 0 | Base `BD30F0` only |

State one covers surface and COM release. State zero is installed before
the normal `resize(0)` call; vector free follows its normal return. The
source uses explicit cleanup state and preserves native termination on a
second C++ exception during cleanup. It does not synthesize releases of
remaining surface fields. A throwing scalar destructor never frees the
group allocation.

For a valid nonnegative capacity, `resize(0)` cannot allocate. The separate
adversarial state-zero fixture sets capacity to -1, causing the full
signed comparison to request reserve zero, which clamps to one 16-byte row.
Any forced allocation failure in that case is explicitly fault injected;
it is not evidence of ordinary valid-owner behavior or a genuine exhausted
allocator.

## Validation scope

The strict MSVC Win32 Release build passed with `/W4 /WX /fp:strict`, both
existing CTests passed, and all eight original seed ranges matched the live
program and installed PE. The ignored source hook adds the owner and the
accepted vector provider without changing shared CMake files.

Seven original-owner/full-library comparisons passed with 2,406 matching
DWORD observations and 88 real COM Release calls. Every observation includes
all 16 normalized group words; construction additionally checks the complete
40h postimage, including untouched padding. Cases cover exact-byte and
unchecked wrapping color assignment, identity, even/odd deletion flags,
current later-field changes, intrusive-terminal and cached-COM exceptions
after a real Release, assignment mutation followed by a throw, and the
separate adversarial state-zero case.

The fixture creates real D3D9 surfaces from a HAL device, constructs actual native
surface owners in the canonical pool, registers them in the actual renderer
view, and uses the full resource-support/string/CRT contexts. Its COM
observer forwards the real Release first and only then mutates fields or
throws. Observer tables are installed after surface construction; forwarding
temporarily restores the real COM table. The malformed-capacity case
substitutes null only after real malloc/free, then invokes the configured
CRT new-handler; this remains controlled fault injection.

Validation results and immutable artifact hashes are recorded in
`reports/native_frame_target_owner_audit.json`. The sole linked `bsp_core.lib`
contains the unchanged built owner, vector, surface, and CRT objects. Full
COFF relocation reconstruction matches linked and before/after runtime bytes
for six owned functions and eight provider entries; the entire fixture
`.text` is unchanged at runtime. Two provider exports are retained as byte
anchors; byte identity alone does not assert a separate out-of-line call.

Original parent bodies are intact except one declared handler-registration
operand relocation. The fixture uses `/SAFESEH:NO` for its private copied
FH3 code and declared ABI bridges to complete vector/surface providers and
CRT/runtime services. The original profile data remains immutable. Fifteen
stages verify complete original postimages, including all unpatched bytes.
Bridged providers are library composition evidence, not original-machine
execution of those provider bodies. No permanent tests were added, and no
game rendering or whole-renderer lifecycle was validated.

The parent renderer binder `B24E70` and the already closed getter leaves
are not part of this six-function packet.
