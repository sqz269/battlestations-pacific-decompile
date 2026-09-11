# Point-effect creation variants (AF)

The complete creation wrappers at `00868420`, `008685E0`, and `008687C0`
now use the existing actual point constructor, admission, intrusive references,
and captured manager lock through `PointEffectConstruction`.
They are additional creation entrypoints used by world, dynamics, and plane
effect callers. The point primary table at `00D0D3EC` has only two entries;
these adjacent functions are not inferred virtual methods.

Evidence comes from the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and the installed executable with SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Names are descriptive hypotheses. Exact ranges, hashes, call sites, signature
audit, and validation records are in `reports/point_effect_creation_variants.json`.

## Original contracts

All three receive the fresh output slot in ECX and return its address in EAX.
The consumed definition is the first stack DWORD and must be nonnull: these
wrappers call `0086A650` unconditionally, unlike the null-definition branch in
`008689C0`. Option and transform arguments are passed as DWORDs but only their
low bytes reach `008680B0`.

| Entry / inclusive range | EDX | Remaining stack DWORDs | Admission input | Constructor matrix |
| --- | --- | --- | --- | --- |
| `00868420..008685D7` (440 bytes, RET10) | opaque third word | matrix, option, tail | original matrix +30h | original live matrix; null parent; transform 0 |
| `008685E0..008687BB` (476 bytes, RET10) | opaque third word | XYZ, option, tail | original live XYZ | current `00F87610`; null parent; transform 0 |
| `008687C0..008689BA` (507 bytes, RET14) | parent | matrix, transform, option, tail | original translation or full matrix product translation | original live matrix; actual parent; third word 0 |

The position wrapper first copies XYZ with three sequential MOVSS stores into
`00F87640/44/48` while holding the captured lock. Input may overlap those
destinations; preserving sequential reads matters. Admission still receives
the original XYZ address. A callback can modify the global matrix before
construction reads it. Native code captures the current-game pointer between
the first source load and the stores, then reads its +19FC reference after
the stores. The typed binding performs its pure reference lookup after the
stores; no arbitrary cross-thread or game-pointer/global-matrix aliasing claim
is made by this interface.

For a nonzero transform byte, the parent-matrix wrapper requires a parent,
refreshes its world matrix through `00B6DB70` if validity bit 2 is absent, and
uses full `00413920` matrix multiplication: original matrix * parent.world.
Only this temporary product's translation is used for admission. Construction
gets the original live matrix and original low transform byte. With transform
zero, admission borrows the original translation directly and parent may be null.

`EffectPointView` borrows three real floats without constructing an overlapping
`std::array<float,3>` inside a matrix. `0086A650` refreshes its reference before
reading the point and passes the same original address to current row virtual
+1C. The existing admission algorithm and base `0086B7D0` AL=1 leaf are unchanged;
their typed interfaces now accept this view. Existing array callers convert
implicitly; implementations overriding the interface must use the view type.

## Ownership and exception evidence

Each wrapper consumes the original input after leaving the captured `00866440`
manager section. Rejection or null 114h allocation writes a null fresh output.
Successful allocation retains an extra constructor argument, calls complete
`008680B0`, publishes the result, retains the output, and releases the temporary
while still holding the lock. The constructor consumes its extra argument on
return or exception. If it throws, the wrapper frees raw point storage before
unlocking and releasing the original input; output remains untouched. Native
point counters are not rolled back by that failure path.

The five-state native maps are:

| Wrapper | Handler | FuncInfo | Unwind map | Input / lock / raw / temporary / guarded-output funclets |
| --- | --- | --- | --- | --- |
| `00868420` | `00C9506C` | `00DC6EF8` | `00DC6F1C` | `C95030 / C95038 / C95040 / C9504B / C95053` |
| `008685E0` | `00C950BC` | `00DC6F44` | `00DC6F68` | `C95080 / C95088 / C95090 / C9509B / C950A3` |
| `008687C0` | `00C9510C` | `00DC6F90` | `00DC6FB4` | `C950D0 / C950D8 / C950E0 / C950EB / C950F3` |

States 0..4 transition to -1,0,1,2,2 respectively. Their actions are guarded
output, original input, captured lock, raw allocation, and temporary reference.
The first two wrappers use EBP-14 for the lock, EBP-20 for the temporary,
EBP-18 for the output flag and EBP-1C for the output slot. The parent variant
uses -54, -60, -58 and -5C. All use EBP+4 for the original input and EBP+8
for the raw allocation. Targets are `0041DE40`, `00411EE0`, `00BF65AC`, and
`00440A30`. These maps explain ordinary C++ unwind order; the original exception
dispatcher was not executed. Nonthrowing intrusive-terminal/free interfaces
do not reproduce all native SEH or throwing-terminal/output-guard behavior.

## Validation and limits

Strict MSVC Win32 build and both existing CTests passed. The focused ignored
fixture executes all 440/476/507 original bytes, including both final RET
instructions and immediate bytes, checks ESP, and compares the C++ wrappers
using the same actual admission/constructor/definition/component/event domains.
It compares complete 114h point images after normalizing only pointer words
+0C/+18/+84/+8C/+110, as well as admission XYZ, constructor matrices, call
counts and current global matrices. Cases include normal construction,
transform zero/nonzero (255), rejection, null allocation, callback mutation,
and overlapping position input. A non-affine matrix element verifies that
parent admission uses the full matrix product.

Direct callees are bridged to canonical C++ implementations and Win32 lock/
Interlocked operations. A manager-layout adapter exposes the actual OS section;
a definition virtual-zero adapter restores its canonical table and dispatches
the actual zero count. Neither adapter proves native-vtable ABI compatibility.
The fixture uses a controlled row predicate to observe and mutate live inputs;
the remaining rumble predicates use the real base leaf. Four named components,
three real 20h rumble events, shared string/node pools and normal terminal
ownership execute through the existing application fixture. The actual nullable
root field is null; parent retention and physical node-pool return are exercised.
Nonnull-root composition, other component families and physical device output
are outside this evidence.

A representative C++ constructor exception verifies unchanged output, raw free
under lock after callee argument cleanup, unlock before original input release,
real component terminal cleanup, and retained native counters. The existing
`008689C0` full-lifetime regression also passed after the view-interface change.
No new permanent tests were added. These are reconstructed C++ entrypoints and
fixture checks; a runnable gameplay-validated rebuild is still unproven.
