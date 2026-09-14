# Native model-group selection

Addresses: `00710BB0`, `00B6DA70`; reused provider `00B6E0A0`.

| Routine | Inclusive native body | Coverage | Native ABI |
| --- | --- | --- | --- |
| `select_native_model_group_00710bb0` | `00710BB0..00710E00`, 593 bytes | complete | ECX actual owner; stack(selected DWORD); RET4 |
| `set_native_node_visibility_factor_00b6da70` | `00B6DA70..00B6DAAB`, 60 bytes | complete | ECX actual node; stack(factor float, recurse DWORD); RET8 |
| Existing `copy_native_node_local_position_00b6e0a0` | `00B6E0A0..00B6E0C0`, 33 bytes | complete, reused | ECX actual node; stack(destination); RET4 |

Names describe reconstructed behavior; they are not recovered symbols. The new
selection interface borrows `NativeModelGroupSelectionAccess*` through EDX and
saves its identity at entry. Native node and model storage remains with its
existing owner. These interfaces require application bindings and are not
drop-in binary replacements.

## Storage and producers

`007135C0` initializes the existing owner+16C/+170/+174 descriptor to zero.
`00713380` takes its descriptor at owner+168 (`007134E4`), forms a zeroed
16-byte inner-vector value (`0071351A..00713520`) and grows it through
`00713270` at `00713523`. The selected row is its begin plus index*16
(`00713548..0071354B`); `00506DD0`, called at `0071355E`, appends the actual
node pointer by writing through row+08 and advancing it four bytes. Its body
establishes row+04/+08/+0C as begin/end/capacity. No duplicate 1ACh owner or
group container is introduced. The sibling construction packet can pass its
existing native allocation directly to selection.

The raw visibility helper uses canonical `NativeNodeStorage` from
`native_node_construction.hpp`. `00B6F5A0` writes first-child+34 at `00B6F5EB`,
next-sibling+3C at `00B6F5F1`, and scalar+AC at `00B6F68E`. The existing GUI
helper remains scoped to its GUI registry and companion transform traversal;
it cannot supply this operation over arbitrary actual native node links.

## Behavior and ordering

Selected zero bypasses the signed-positive/range gate; other values must be
signed positive and below the unsigned result of `(end-begin) SAR 4`. A null
begin or zero count returns. Subsequent comparisons, including corrupt
negative differences, retain the native unsigned interpretation.

The routine first walks every unselected group, then the selected group. It
retains each current inner descriptor and iterator while reloading actual
bounds at each native comparison. Every outer iteration reloads owner+16C
and +170; the final selected row reloads owner+16C. Invalid-parameter calls
retain their continuations: BF6713 passes five zero arguments to BF66EF and
cleans 14h bytes at BF671F. BF66EF can return through its decoded configured
handler, so a noreturn assumption would remove native behavior.

Unselected nodes are changed only when MOVSS/UCOMISS of node+AC equals the
current float at D7A24C. Selected nodes compare against current D7A218.
LAHF/TEST AH,44h/JP preserves unordered behavior and MXCSR exception handling.
The helper arguments are produced with FLDZ or FLD1 followed by FSTP to the
original stack slot; they are not copies of the comparison globals.

The reused getter copies actual node+E0/+E4/+E8 through its original FLD/FSTP
sequence. Hiding adds the live **double** at CF81F0 to y and x87 positive zero
to x/z; showing subtracts it and x87 zero respectively. Installed CF81F0 is
100000.0, D7A24C is 1.0f, and D7A218 is +0.0f. Current values are read from
borrowed locations at the original instruction sites. No finite-value,
rounding-mode, exception-mask or immutable-global assumption is added.

Current node virtual+2C is captured at the native moment, including before
the selected branch's y float spill. The dispatcher receives that exact
target in EDX, the actual node in ECX, and the original position pointer on
the stack. It must invoke the supplied target through the real native
binding. It may mutate live storage; the caller retains every subsequent
native reload. The source adds only a saved access pointer and explicit
global/call adapters around the original instruction sequence.

The raw visibility operation initially copies all factor bits with MOVSS,
tests only the recurse DWORD's low byte, walks actual first-child+34, and
reloads the current sibling+3C after each recursive call. Each recursive
factor passes through FLD/FSTP, preserving SNaN and unmasked-x87 behavior.

## Call and verification evidence

`reports/native_model_group_selection.json` records all 17 outgoing calls,
including the two indirect setter sites, and all incoming xrefs: seven to
selection and 199 to visibility. Live body attribution exists for 182
incoming sites. The other 24 visibility references have no enclosing Ghidra
function; the report records each exact five-byte call span with inclusive
endpoints under `no_ghidra_function`. No enclosing body is inferred.

All Ghidra operations were read-only against `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, with target verification on every BSP query.
Existing Ghidra names/comments were preserved. The owned bodies and three
constant spans matched the installed PE bytes after seed verification.

MSVC Win32 `scripts/build.ps1` passes, including both existing CTests.
The focused original-byte/source probe passes eight groups, 23 variants:

- Twelve x87 precision/rounding combinations, including SNaN and signed zero.
- Mutated outer storage, retained inner descriptor/bounds, live constants,
  and current virtual target; returning checked-iterator handler.
- Negative/high-bit/out-of-range selected gates; masked/unmasked UCOMISS SNaN.
- Unmasked x87 overflow with the current virtual target captured before the
  selected y spill; recursive raw hierarchy SNaN, low-byte recurse, and
  unmasked recursive FLD.

The probe compares complete fixture bytes, call events, trap counts, exception
codes, x87 control/status and MXCSR. All 46 original/source result images are
retained: eight DWORDs (fixture size, event-word count, traps, exception,
x87 status, x87 control, MXCSR, target-captured flag), then fixture bytes,
then 48 event DWORDs. They are fixtures over actual-format storage; they do
not establish game behavior or exhaustive exceptional-input equivalence.

The compiled audit checks all 179 original instruction sites, every branch
destination, direct-call COFF relocations, and each explicit adapter. The
640-byte selection code is identical between the probe object and CMake
object. Visibility's 60 bytes and the canonical getter's 33 bytes match
native code after normalizing the visibility recursive-call relocation.
For the selected arithmetic block, compiled FSUB is at 224h, current vtable
loads at 227h/229h, and FSTP at 22Eh. The unmasked-overflow case independently
checks that ordering at execution time.

Ignored `local/model_group_*` evidence includes original bodies, live
attributions/listings, byte relocation proof, source and compiler listings,
objects, executable with embedded manifest, run outputs, paired snapshots,
and exact compiler/header/SDK/library hashes. The integrator must archive
`model_group_review_manifest.json` inputs and artifacts before worktree
cleanup. Full game execution, arbitrary application dispatch targets and
CRT Watson termination are not validated here.


## Integration review, batch AA

Correction from `docs/ORCH6_RECONSTRUCTION_AA.md`: The integrator verified all197 direct rows, archived the609 manifest hashes, saved confirmed names/evidence and refreshed exports. The24 raw incoming call spans retain their worker-recorded missing function ownership; they are not inferred functions or reconstructed bodies.
