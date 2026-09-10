# Material point-light constants

`write_material_point_light_constants_00b43160_fragment` reconstructs the final
point-light branch `[00B43160,00B43403)` of `00B42350`. The existing
`CompiledMaterialPass.vb` metadata selects writes into the caller's actual shared
vertex constant vector. The branch does not access pixel metadata or constants.
It is a bounded C++ interface, not the complete material builder or a native ABI
replacement.

## Shared owner and call site

At `00B43160`, EBP is the render entry and EBX is the current material pass.
`[entry+0C]` supplies the actual model to `00B6DC50`. That complete seven-byte
helper is `LEA EAX,[ECX+164]; RET`: it returns the borrowed array header without
copying, retaining or releasing a light. The containing function's original ABI
is ECX pass, stack entry and override, `RET8`; the fragment has no separate ABI.

`borrowed_point_lights_00b6dc50(model)` returns a reference to the same
`GeneratedModelLifetime.point_lights_164` vector used by lifetime unlink and
instance consumers. Its `GeneratedModelPointLightLinks` entries preserve actual
borrowed light identity, `BuildingInstancePointLight` values, and reciprocal
borrowed model links. This is distinct from model-owned geometry at `+180`.
The integrator must resolve this live model/list at the original stage after
preceding callbacks and helpers, rather than capture a list at builder entry.

The owner interpretation follows the current evidence in
`reports/model_point_light_owner_review.json` and the existing
`include/bsp/generated_model_lifetime.hpp`. This packet does not introduce light
ownership or a second light collection.

## Native writes

The current registry `src/shader_system_registry.inc` identifies semantic45 as
`cPointLightCount` and semantic46 as `cPointLightsData`. They correspond to the
native VS metadata bytes `+35` and `+36`, respectively. The existing reflection
mapping supplies `CompiledMaterialPass.vb.registers`; this writer does not infer
registers from a shader name, reflection order, register counts or end register.

| Native address | Recovered behavior |
| --- | --- |
| `00B43168..00B43171` | Read count register from `[pass+70]+35`; `FF` skips the entire branch. |
| `00B43177..00B4319F` | Read list count once; signed counts at least4 become4; convert to float. Valid owner counts are0..INT_MAX. |
| `00B431AF..00B431BB` | Write count register lanes in order Y=+0, Z=+0, X=count, W=+0. |
| `00B431C0..00B431CD` | Reload VS metadata and read data register `+36` after count writes; `FF` skips data. |
| `00B43200..00B4338D` | Unrolled four-light path, in actual list order. |
| `00B43391..00B43401` | Residual path for one to three lights; zero count performs no data writes. |

For each selected light `i`, register `data+2*i` receives position/radius from
native light fields `+1EC/+1F0/+1F4/+1F8`; register `data+2*i+1` receives color
from `+184/+188/+18C/+190`. All four position words load before any position
store. Color words then load and store individually in lane order. Each next
light pointer is fetched from the same list only after the preceding record.
The C++ implementation uses integer-word copies to preserve source bits,
including NaN payloads, signed zero and both W components.

Count and data registers may overlap. Data runs later and overwrites any
overlapping count lanes. Other constants and unused light records retain their
previous contents: there is no whole-buffer or unused-tail clear. This differs
from `write_building_instance_data_00b55780`, which takes three lights, separates
position and color groups, clears absent records and replaces color W lanes.
That instance output cannot supply this branch.

## Checked interface and limits

The function borrows `const CompiledMaterialPass&`, the actual model's light
vector by reference, and the caller's shared `std::vector<float>&` output. It
never resizes the output or stages the whole result. Current metadata is read at
the original stages; selected light values are read in native order. The
metadata/list headers and source objects must retain valid C++ storage. The
typed interface does not project corrupt native pointer/header aliases into
C++ containers.

New bounds and null-owner failures prevent undefined host accesses; they are
not native error paths. An invalid signed-domain count or short count register
fails before writing. A short selected data range fails after the count write.
A null selected owner fails after the count and any preceding light records.
Missing count metadata returns successfully without inspecting list count,
light pointers or output capacity. Missing data metadata or zero count avoids
data-range validation. Reflection `counts` and `end_register` are unused.

Native corrupt negative DWORD counts take an unsigned conversion/iteration path
after the signed cap and can traverse an enormous invalid list. That behavior,
invalid native pointers, exact floating-point exception flags, and simultaneous
unsynchronized container mutation are outside this valid-owner interface.
Counts0..4 convert exactly without depending on x87 extended precision.

## Evidence and validation

Every live query used `python tools/bsp.py ghidra ...`, whose client verifies
project `bsp`, program `/battlestationspacific.exe`, language and image base
before accessing the existing `C:/Users/sqz269/bsp.gpr` project. All675 assigned
branch bytes and all7 helper bytes matched the installed PE, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The audit records full bytes, range hashes, ABI, existing function/comment
preimages, and append-only annotation proposals. This worker made no Ghidra or
shared-ledger changes.

`python tools/ghidra_export.py verify-seeds` passed, and `scripts/build.ps1`
passed both existing reconstructed/native math checks. The new source separately
compiled and linked against the built library using MSVC Win32, C++17,
`/O2 /W4 /WX /fp:strict`. Primary CMake integration remains with the integrator.

The ignored `local/point_light_native_fixture.cpp` executes the verified branch
and helper with only the helper-call displacement and three absolute VS address
operands relocated. The parent epilogue, outside the assigned range, is replaced
by a fixture return. Nine full-VS-buffer comparisons passed: counts0/1/3/4/5,
missing count/data registers, and two count/data overlaps. Unique untouched
buffer tails, signaling-NaN color payloads and negative-zero position W bits
matched exactly. Pixel canaries remained unchanged; disassembly independently
shows no PS accesses. One checked-host case verified that late data bounds
failure preserves the count write. No broad tracked test suite was added.

These are reconstruction, strict compilation and isolated native-fixture
results. They do not establish native object layout/ABI compatibility, complete
material-builder execution, live device upload, visual parity or game behavior.
