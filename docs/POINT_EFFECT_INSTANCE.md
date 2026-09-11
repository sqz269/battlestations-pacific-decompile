# Point-effect instance constructor fragments

`008680B0` is the `0x114` instance constructor required by
`create_point_effect_008689c0`. Three substantive stages now execute against the
actual allocated instance storage. The complete constructor remains required:
these stages do not make allocation, entry creation, manager insertion, or full
exception cleanup succeed by assumption.

## Evidence and ABI

Read-only Ghidra queries verified project `bsp`, configured project file
`C:/Users/sqz269/bsp.gpr`, and program `/battlestationspacific.exe`. The project
file exists. `bsp.py` re-verifies project/program, x86 language and image base
before every live analysis/export query. Exports are in ignored
`exports/bsp/functions/008680b0` and `008672a0`. The saved constructor remains
`FUN_008680b0`; annotations are the integrator's responsibility.

Native ECX is raw instance storage; seven stack DWORD slots are template,
parent, third word, matrix pointer, transform byte, option byte, and tail word.
The low byte is used for the two byte arguments. EAX returns the same storage;
`RET 1Ch` starts at `008683D4` and ends at `008683D6`. The body has 232 listed
instructions and no gaps. Assembly is necessary: the decompiler loses receiver
arguments, the matrix destination, and callback-sensitive reloads.

`PointEffectInstanceStorage` is exactly `0x114` bytes on MSVC Win32, with checked
field offsets. Its pointers refer to canonical host companions that borrow the
actual native fields; integer vtable identities `CEB130` and `D0D3EC` are never
called. Thus the representation is a new C++ interface, not a drop-in ABI.

## Executable stages

| Native interval, inclusive | Recovered operation |
| --- | --- |
| `008680D9..00868192` | Base/derived identity stores, actual count initialization, scalar and two empty array initialization, separate template-member retain, ordered global increments. |
| `008681BE..0086824C` | Store an already constructed node, retain its actual count, conditionally propagate its current root using canonical `B6D890`, then release/replace/retain the effect parent. |
| `0086826E..008682D4` | Capture current node, refresh world if bit2 is clear, snapshot16 DWORDs, canonical x87 matrix copy into instance+90, initialize remaining scalar fields before array resize. |

The first stage writes `+04=1`; option at `+08`; zero at `+09,+0A`; empty
pointer/count/capacity arrays at `+0C` and `+18`; tail word at `+24`; ones at
`+28,+2C`; positive zero at `+30..44,+58,+80`; and `1.0f` at `+50,+54` from the
original `D7A24C` word `3F800000`. It first clears `+84`, then publishes and
retains the nonnull argument there. It clears `+88,+8C`, increments `F87604`,
then increments `F87600`, using unsigned modulo32-bit arithmetic. It does not
give provisional scalar fields a meaning the evidence does not establish.

Bytes `+0B`, `+74..7F`, both matrices, `+110`, and the later initialized scalars
remain their allocation preimage at the end of this first stage. The cache
stage writes `+4C` with exact native `D7A238` bits `3C23D70A` (`0.01f`), then
positive zero at `+48,+5C..70`. The native first copies world with `REP MOVSD`
into a stack matrix and only then calls x87 `4134F0`; the implementation keeps
that bit-copy snapshot and x87-copy distinction.

The node stage requires successful real `B6ED70(174h)` allocation and
`B6F5A0(template+1C)` construction to have already happened. The latter has an
existing canonical constructor and stable `NativeNodeBinding`. This stage
does not allocate a substitute node or silently convert the native null pool
return into success: the original subsequently increments `[node+4]` even when
the allocation branch returned null.

Root registration happens when either the third stack word or requested parent
is nonzero. It reads current `[E188A8]+19EC` and invokes the actual canonical
node/scene registration runtime. It then reloads instance+8C, releases that
parent's actual reference, clears+8C **after** its terminal callback, publishes
the requested parent, and retains it. In particular, registration may have
installed a parent even though initialization set+8C to null. Parent references
and their terminal actions must remain valid through native reentry; no new
reference counter or hierarchy is created. The effect parent at+8C is distinct
from the constructed node's hierarchy parent at node+30.

## Separate argument ownership and exception evidence

`PointEffectTemplateArgument` consumes exactly the incoming reference without
retaining it. Keep that scope alive through the complete future constructor,
and run the actual partial-member unwind before its destructor on exception.
The instance+84 retain belongs to a different lifetime. The fragments do not
release that member, unwind a constructed owner, or return physical storage.

The normal argument release is `0086839A..008683BE`, after manager insertion.
The handler starts at `00C95022`; it loads `DC6E94`, then jumps to `BF6B43` at
`C95027` (inclusive end `C9502B`). Its eight-state unwind map at `DC6EB8` is:

| State | Next | Funclet | Operation |
| --- | --- | --- | --- |
| 0 | -1 | `C94FD0..C94FD7` | Incoming template stack slot via `41DE40`. |
| 1 | 0 | `C94FD8..C94FDF` | Reference base via `BD30F0`. |
| 2 | 1 | `C94FE0..C94FEA` | Array+0C via `8675B0`. |
| 3 | 2 | `C94FEB..C94FF5` | Array+18 via `8675B0`. |
| 4 | 3 | `C94FF6..C95003` | Member+84 via `41DE40`. |
| 5 | 4 | `C95004..C95011` | Parent+8C via `605FD0`. |
| 6 | 5 | `C95012..C95019` | Raw node allocation via `B6E670` if its constructor throws. |
| 7 | 5 | `C9501A..C95021` | Temporary entry result via `6CF070`. |

`8675B0` resizes its array to zero with `8672A0`, then frees the backing pointer.
`605FD0` releases and then clears its pointed parent slot. The map contains no
separate constructed-node+110 cleanup or `F87604/F87600` rollback. Do not invent
either as part of a future implementation. The fragments leave partially
initialized state visible when the canonical registration operation throws.

## Remaining constructor dependencies and ordering

After initialization, the original allocates and constructs a node, performs
the recovered registration/parent stage, and selects `53D9C0` for transform
byte0 or `72AA80` for nonzero. Both setters still require actual node virtual+34.
`53D9C0` invokes that virtual with the original matrix, then reloads parent+8C;
with a parent it computes input times parent inverse-world into relative+D0,
otherwise copies input. `72AA80` first copies input into+D0, captures parent,
refreshes its world if needed, then sends relative times parent-world (or+D0
without a parent) to node virtual+34. These boundaries are not replaced by a
success callback or an integer-vtable cast.

After the cache stage, `8672A0` resizes entries+0C to the **current** template+0C
count. It is a reference-array resize, not a base constructor. Growth requires
`8670A0`; new slots initialize null; shrink decrements the live count before
releasing each removed pointer and clears the slot after terminal release.
Its actual backing allocator, overflow/failure, and reference cleanup still
need the full vector contract.

The constructor then refreshes the current+110 node and current
`[E188A8]+19FC` reference transform. If instance option+08 is zero it captures
template+08 begin and template+0C end exactly once, iterates the original slots,
and calls current row virtual+18(instance) only when row bytes+10 and+1C are
both nonzero. After each virtual call it reloads the instance entries backing
pointer, assigns the returned owned reference into that index using `6FBEB0`
(matching canonical publish/retain/release assignment), then releases the
temporary result. Mutation must preserve the captured template extent and
currently addressed output storage. Full row factory dispatch is not supplied.

Finally `4D1100` obtains the instance manager and `867500` inserts the instance.
`867500` additionally gets `866440`'s lock and uses `74D780`, with reference
decrement/possible terminal action and subsequent increment before unlocking.
This must be recovered with the real manager/list and failure behavior; it is
not the already reconstructed eight-byte `866440` lock-owner singleton.
Only after that call does the constructor consume its argument and return.

## Validation

Direct MSVC Win32 `/std:c++17 /O2 /W4 /WX /fp:strict /MD` compilation passed.
One ignored fixture, linked with `/MANIFEST:EMBED`, passed actual count and
template argument lifetime checks, allocation-preimage preservation, ordered
root callback/parent replacement including a reentrant terminal callback,
canonical node-world refresh/cache and exact scalar bits, and consumed argument
unwind after explicit member cleanup. This fixture verifies the bounded host
operations, not full native constructor unwind or manager insertion.

`scripts/build.ps1` passed both existing CTests, `reconstructed_math` and
`native_math_differential`. The new source is compiled directly because the
integrator owns `cmake/startup.cmake`; it needs registry integration there.
No permanent tests were added. No game execution, installation modification,
ABI-compatibility claim, or game validation was performed.
