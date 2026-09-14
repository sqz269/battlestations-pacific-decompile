# Actual D3D9 shader construction and unwind

Addresses: `B5F9B0`, `B5FAF0`, `B289A0`, `B289F0`, `B22DD0`,
`B22E30`, `B5E720`, `B5E7E0`, `B3F4C0`.

The existing shader constructors now have an operation-free route through the
application's actual `01090AA0` manager and `01090AA8` string pool. The original
and new interfaces share the complete constructor and registry bodies. The
legacy construction context stores `SoundLifetimeAccess`, which still accepts
existing `SingletonLifetimeDomain&` aggregate initializers. Its explicit
operation interface retains its previous failed-operation and replay guards.

`NativeD3d9ShaderConstructionActualContext` borrows `NativeStringRawPoolContext`,
`0108FEDC` support publication and current `00F8D394` renderer publication. The
pool context supplies the same actual AA0 and AA4 cells. No new manager, owner
projection, callable profile table, admission-time getter, operation allocation,
or successful fallback is introduced. Real COM vslot4, existing raw string/pool
providers, raw manager registration and actual renderer array storage execute.

| Entry | Bytes | Coverage | Source change |
| --- | ---: | --- | --- |
| B5F9B0 pixel / B5FAF0 vertex | 320 each | Complete | Shared normal body plus actual AA0 and native cleanup schedule |
| B289A0 vertex / B289F0 pixel | 73 each | Complete | Operation-free shared registry overloads |
| B22DD0 vertex / B22E30 pixel | 95 each | Complete | Operation-free shared reserve overloads |
| B5E720 pixel / B5E7E0 vertex | 5 each | Complete | Concrete tail composition to existing BD30F0 |
| B3F4C0 diagnostic record | 30 | Complete | Raw-pool overload of existing diagnostic-record contract, composed through raw 41DD20 at record+4 |

The original constructors receive the fresh actual 10h owner in ECX and COM
argument on the stack, return the same owner in EAX and execute RET4. Registry
entries also consume one stacked argument. The base thunks and diagnostic record
use ECX/RET. The new context-taking C++ interfaces are not binary replacements.
The two base wrappers retain the ECX/RET calling shape.

The constructor stamps CEB130 and stores count1 before arming base cleanup. It
then stamps D62A60/D62A70, clears +0C and +08, publishes captured incoming COM,
and calls its current vslot4 AddRef. EDI is literal zero across that ABI-preserving
call; the listed old-COM Release sites B5FA14/B5FB54 are unreachable. Acquired
COM is not released by constructor cleanup. +0C has no inferred semantic role.

Vertex registration reloads F8D394 before either temporary string. Pixel
registration reloads it only after both normal pool returns. Registry removal
uses the existing full B253E0/B25450 actual pointer-array bodies. It removes the
first match by swapping the captured last element, then decrements current count.
Registration grows only at current count==captured capacity, with signed
max(wrapped capacity*2,1), and appends the borrowed wrapper. Reserve clamps the
signed request to1, reloads current signed count and base while copying, frees
current old base, then publishes new base and capacity. No pointee ownership or
rollback is added. The raw overload allocates no diagnostic child operation.

The first string is resized to11/12 and copied with captured length+1. Then the
current owner+08 is saved into the borrowed COM word of a 0Ch temporary; it is
not an argument to B3E730. The second string is resized with captured first
length, and its copy reads current first data and current second length. The
native BF7680 provider admits overlap, so these copies use `std::memmove`.
Normal cleanup returns captured second data with current second length+1,
followed by current first data with captured first length+1. Each nonnull return
resolves the actual current pool before BD1510, even with the small-return gate
set. Pointer captures occur before that getter.

| State | Pixel action | Vertex action | Receiver and next state |
| ---: | --- | --- | --- |
| 0 | CC1220 -> B5E720 | CC1250 -> B5E7E0 | Captured owner [EBP-24], stamp base only, then -1 |
| 1 | CC1228 -> 41DD20 | CC1258 -> 41DD20 | First header EBP-20, then0 |
| 2 | CC1230 -> B3F4C0 | CC1260 -> B3F4C0 | Support temporary EBP-18; string starts EBP-14, then1 |

Pixel FuncInfo DF9F88/map DF9F70 and vertex DF9FC4/map DF9FAC each contain these
three states. State1 is armed only after the first resize and copy; state2 only
after the second resize and copy. Normal cleanup disarms each state before the
pool return. Source C++ exceptions therefore clean only completed temporaries
and the base. They leave unarmed allocations, acquired COM, completed vertex
registration and early support publication untouched. The caller still owns the
shader allocation. No failed operation is acknowledged or disarmed to imitate
this native unwind. A second exception from cleanup during C++ unwind terminates;
original private FH3, collided-unwind and SEH behavior are not established.

B3F4C0 already had a complete NativeStringStorage/noexcept implementation in
`native_physical_buffer_owner.cpp`. That interface is preserved. The new overload
uses the existing raw 41DD20 body at record+4, with the same current+08 pointer,
current+04 length and untouched borrowed COM/header. This permits raw getter
failure without imposing the older storage interface's noexcept boundary.

Evidence is retained in `reports/native_d3d9_shader_construction_actual.json`.
Fifteen complete live/installed-PE spans match across1,435 bytes. All nine owned
functions have zero listing gaps. Thirteen full instruction blocks pin receiver,
store, capture, wrapping and argument order; all six cleanup-map receiver rows
are checked against bytes. The numeric gate covers40 direct calls/tails, with
four indirect COM sites separately documented. The primary
agent subsequently defined the complete10-byte handlers CC1238..CC1241 and
CC1268..CC1271 through the official tool, saved, and forced exports. Their
FuncInfo/tail operands now enter the numeric gate. The primary mutation record
is attributed separately; this worker made no Ghidra mutation.

Validation uses the strict Win32 build, eight verified native seeds, both
configured CTests, and one manifested `/MD` probe. The probe compares copied
original normal pixel/vertex constructors and registry helpers with source using
real HAL shaders and the actual AA0/AA8/FEDC providers. A source-only exception
from real CRT validation after support publication checks both temporary returns,
base stamp, captured section release, retained COM and vertex registration.
Legacy retained-operation failure and replay/retirement guards are checked in
the same probe. Original handlers are never executed. The failed legacy frame
and its borrowed providers remain alive in a process arena; diagnostic retirement
is not counted as native cleanup.

The immutable manifest records final source, compiler-read headers, selected
library inputs, fixture/executed original bytes, build/evidence tools, installed
PE and actual loaded module paths resolved inside the32-bit probe. These results
do not close the effect compiler tail, its caller allocation cleanup, full shader
owner graph, renderer lifecycle, draw/readback or gameplay. Runtime tests do not
cover every earlier allocation failure or failure from a normal cleanup getter;
those arm/disarm and preimage contracts are assembly/source evidence.

## Integrated validation at 56780ba7

Original/source pixel and vertex normal construction matched with real HAL shaders, actual AA0/AA8/FEDC publications, registry operations and canonical drain. Source-only postpublication failures checked armed cleanup order, base stamp, retained COM/vertex registration/publication and unlocked captured section. Legacy retained-frame checks passed, including expected destructor guard exit77. Original FH3/SEH execution remains unproved.

The combined strict Win32 build, eight seed checks and both CTests passed.
Three final-library probes,136 numeric direct/tail rows,23 saved/read-back
annotations (20 source entries and3 handler contexts), and70 live/PE spans
are retained in `local/checkpoints/56780ba7/native-shader-parent-wave/validation.json`
(SHA256 `2c757704e58574665b12a4cfcc05ea693b880e1923a1e1daf67031202d633065`). Full renderer and gameplay validation remain open.
