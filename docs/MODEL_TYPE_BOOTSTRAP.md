# Native model type bootstrap

`ModelTypeBootstrap` reconstructs the five complete type entries used by the
native `c3dObject` model family. It binds the actual byte guard at `01090030`
and four DWORDs at `01090034`: own ID, node ID, root ID, and native name address.
`ModelTypeDescriptor` is exactly 16 bytes, with offsets `0/4/8/C`; its POD fields
have no default initializers. Constructing the wrapper does not initialize or
reset the bound process storage.

The supplied `TypeIdCounterLifetime` must be the same instance used by the
supplied `LightTypeBootstrap`, including the actual `0109DB7C` pointer slot and
shared singleton lifetime domain. This is a caller precondition, following the
existing camera bootstrap interface: the light API does not expose its counter
identity for validation. Node storage is obtained directly from
`LightTypeBootstrap::storage().node_0108ff90`. The implementation calls the
existing `initialize_node_00b6f110` on that descriptor, which owns the shared
root/node guards and root initialization. It creates no counter, descriptor copy,
or separate lifetime domain.

| Entry and complete extent | Original ABI | Reconstructed method |
| --- | --- | --- |
| `00B74330..00B74335`, 6 bytes | ECX ignored; EAX live `[01090034]`; `RET` | `type_id_00b74330` |
| `00B74340..00B74345`, 6 bytes | ECX ignored; EAX live `[01090040]`; `RET` | `type_name_address_00b74340` |
| `006EF860..006EF887`, 40 bytes | ECX ignored; stack token; AL boolean; `RET 4` | `is_type_006ef860` |
| `00CD7E60..00CD7EAE`, 79 bytes | No arguments; no defined result; `RET` | `initialize_static_00cd7e60` |
| `00B74F90..00B74FD6`, 71 bytes | ECX four-word target; no defined result; `RET` | `initialize_00b74f90` |

The three leaves always read the current canonical descriptor. The predicate
compares the own, node, and root words in address order, stopping at the first
match. It does not include the name word or any light token. No leaf calls an
initializer or checks the guard: token zero therefore matches a zero-filled
descriptor before initialization, and later direct writes are visible on the
next call. Only AL is a defined native predicate result. The name getter returns
a native address word, not a host string pointer.

Both initializers test the process guard and return without touching the target
when it is any nonzero byte. On the cold branch, they publish guard `1`, store
name address `00D62DD4` (`c3dObject`), and call the actual shared node initializer.
The static entry then loads both node ancestry words before either destination
store (`00CD7E84..00CD7E94`). The lazy entry instead loads node-own, stores target
`+4`, loads node-root, and stores target `+8` (`00B74FB4..00B74FC2`). This ordering
difference remains explicit in the C++ and matters for overlapping native data.

Each cold branch calls `TypeIdCounterLifetime::get_006fac20`, captures counter
`+4`, stores the increment with unsigned 32-bit wrapping, and only then publishes
the captured own ID. The static decompiler presentation suggests the opposite
store order; assembly at `00CD7EA5/00CD7EA8` establishes the order used here.
The lazy entry takes its destination from ECX while retaining the process-wide
guard. Initializing an alternate target can thus leave the canonical descriptor
unchanged while suppressing a later canonical initialization. Neither failure
rollback, guard reset, additional locking, nor atomic operations are added.

## Evidence and validation

The five complete body ranges and `c3dObject` string independently matched the
installed executable after each `bsp.py ghidra bytes` call verified
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. The executable SHA-256
is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Assembly was inspected through the final return for every entry. The initial
discovery found no Ghidra function definitions at `00B74330/00B74340`; primary
integration owns creating those definitions and saving annotations and exports.

One ignored local MSVC Win32 differential sequence executes the original five
entries and original `00B6F110/006FAC20` helper instructions against the C++.
Seven code ranges and five data ranges were verified against saved analysis and
installed bytes or PE zero-fill. The isolated executable image uses 35 explicit
pointer relocations; relative calls remain unchanged. No helper implementation
is substituted. The shared node guard is pre-set and the counter prepublished,
so the original node guard-return and counter fast path execute.

The sequence checks live leaves before initialization and after descriptor
mutation, static initialization with `FFFFFFFF -> 0` counter wrap, and one
deliberate Win32 raw-storage overlap: lazy target equals the shared node while
counter `+4` equals target own ID. Matching results establish the interleaved
lazy parent copy and counter increment-before-publication order. Canonical
getter isolation and suppression with guard `80` are checked afterward. One
explicit fixture-only guard reset permits both cold model entries in this
sequence; that reset is absent from production code. Native name words are
normalized only by the fixture image relocation delta. The two six-word
checkpoints agree exactly. The raw overlay probe is specific to target MSVC
Win32 storage semantics and is not a portable C++ object-layout claim.

The module and fixture pass `/std:c++20 /W4 /WX /fp:strict`. The worker ran
`scripts/build.ps1` with this source added through an ignored local CMake hook;
the existing `reconstructed_math` test passed (1/1). Native seed verification
passed all eight existing spans. No repository test cases were added.

Cold root/node initialization, counter allocation and registration, dependency
failure or reentry, SEH/unwind, and concurrent initialization were not executed
by this focused fixture. The guard-before-dependency order and static capture
order are supported by full assembly inspection. Existing dependencies retain
their own validation evidence; this packet does not broaden those claims.
The C++ names are descriptive interpretations, and these are new typed interfaces,
not drop-in native ABI replacements. Model owner integration, process startup
ordering, game execution, and visual behavior are outside this packet. CMake,
shared ledgers, Ghidra annotations, and export refresh belong to the primary
integration pass.
