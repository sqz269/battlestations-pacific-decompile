# Application ordinary construction chain

All three complete ordinary bodies are established: `737970` is **42 bytes /
15 instructions**, `BEA970` is **22 bytes / 8 instructions**, and `BEA810` is
**145 bytes / 40 instructions**. They preserve the original receiver identity,
publish and register before outer fields are initialized, and directly write
24 of the known 28 owner bytes. This remains evidence for a future genuine
Application owner; no Source constructor, callable table or new credit is added.

The [report](../reports/cc12_application_ordinary_construction_readiness.json)
retains all 209 fresh bytes, complete live listings and independent decodes,
exact owner stores, service bindings, Source pins and explicit limitations.
The starting main was `7e4d26f1b550d623d1cba8304fbefd1dad0f798d`.

## Scope and receiver identity

The existing `bsp` project, `/battlestationspacific.exe`, and unchanged function
count **64729** were verified before analysis. The configured saved project is
`C:/Users/sqz269/bsp.gpr`. Each body passed its own 300-byte gate before complete
inspection. No other native caller, child service, handler, funclet or table was
queried. The two import operands were resolved from existing PE import metadata.

The prior producer audit supplies the same WinMain stack-owner context and its
28-byte extent. This packet does not re-audit the caller's entire stack layout;
the highest direct constructor byte store at `+1A` alone is not an extent proof.

All three entries consume actual `ECX=A`, accept no explicit stack argument,
return the original `A` in EAX, and execute normal `RET0`:

- `737970` saves A in ESI and passes unchanged ECX to `BEA970`. It explicitly
  discards child EAX with `XOR EAX,EAX`, then uses its original ESI for every
  owner store and final return.
- `BEA970` likewise saves A in ESI and passes it to `BEA810`. It ignores the
  child's returned EAX, writes through its original ESI, and returns that ESI.
- `BEA810` saves A in EDI and explicitly returns it after all manager,
  registration and lock operations. It does not return the current publication
  or a service result.

No body allocates the outer owner, adopts a relocated child result, or guards a
null receiver. Actual storage and callee-saved register/stack contracts must
remain valid through the reached stores and normal return. Placeholder saved
`undefined(void)` prototypes do not remove the physical input/result.

## Publication and base-construction schedule

| Site | Exact normal operation |
| --- | --- |
| `BEA810..BEA830` | Install SEH frame with `CC7240`; capture A in EDI and a local spill; set whole local state DWORD to zero. |
| `BEA838` | Write `D68BC4` to actual A+0. |
| `BEA83E..BEA850` | Get manager M1; capture S=`[M1+10h]` in ESI without a manager-null check; materialize local guard words `CE37FC` and S. |
| `BEA854..BEA85D` | If S is nonnull, enter S, then increment its current DWORD `+18h`. |
| `BEA861..BEA866` | Set low state byte to one; unconditionally publish original A into actual `E1AE90`. |
| `BEA86C..BEA87A` | Get manager M2 separately; **after it returns**, reload current `E1AE90=P`; push P, set ECX=M2 and call `BD0C30`. |
| `BEA87F..BEA888` | If original captured S is nonnull, decrement its current DWORD `+18h`, then leave that same S. |
| `BEA88E..BEA8A0` | Restore exception-chain/register/stack state, set EAX=A and return. |

`CE2218` resolves to `KERNEL32!EnterCriticalSection`; `CE2210` resolves to
`KERNEL32!LeaveCriticalSection`. The counter operations are ordinary wrapping
DWORD ADDs. There is no manager/lock refresh or stronger atomicity guarantee.

M1 and M2 need not be substituted for one another: the first supplies the
captured section, the second supplies the registration receiver. The body does
not compare their identities. Similarly, it registers current P after the
second getter, which can differ from A or be null. The raw registration service
validates manager bounds before its own null-object check.

Publication occurs while A has the `D68BC4` profile, before the later loop flag,
vector, Game and outer flag stores. These fields have not yet been directly
initialized by the chain and remain subject to caller preimage and external
effects. Services and registration can observe that construction stage.

The publication write has no old-value check or old-owner release. There is no
post-registration publication reload or forced `E1AE90=A` restoration. All
three constructors still return captured A if an external service changes the
publication. Registration adds no owner retain/decrement or duplicate policy.

## Exact owner write map

After `BEA810` returns, `BEA970` writes `D68BC8` and zeroes only byte `+4`.
After that child returns, `737970` writes `CFEAB0`, then DWORDs `+8`, `+C`,
`+10`, `+14`, then bytes `+19`, `+1A`, and finally byte `+18`.

| Owner bytes | Direct writes in this chain |
| --- | --- |
| `00..03` | `D68BC4` at `BEA838`, then `D68BC8` at `BEA978`, then `CFEAB0` at `73797A`. |
| `04` | Byte zero at `BEA97E`. |
| `05..07` | No direct store; caller preimage not established. |
| `08..0B` | Data DWORD zero at `737980`. |
| `0C..0F` | Signed count DWORD zero at `737983`. |
| `10..13` | Signed capacity DWORD zero at `737986`. |
| `14..17` | Game pointer DWORD zero at `737989`. |
| `18` | Byte one at `737992`, the last owner store. |
| `19` | Byte zero at `73798C`. |
| `1A` | Byte zero at `73798F`. |
| `1B` | No direct store; caller preimage not established. |

The 11 owner-store instructions write 32 byte positions counting repeated
table stores, covering 24 distinct owner bytes. The four remaining bytes are
not directly changed by these bodies. This does not prove that callbacks or
concurrent writes through aliases cannot affect them. It does not authorize a
whole-object zero fill or a widened DWORD store at `+4`.

The actual vector header is the existing 12-byte storage at owner+8, with data,
signed count and signed capacity. It is an interior component, not the complete
Application owner. `ApplicationFrameState` is also explicitly a projection.

## Concrete existing Source services

The two observed `415350` edges can reuse
`get_native_singleton_manager_00415350` with the actual canonical manager cell.
`GameNativeStringProcess` owns that process-retained cell, and
`GameSingletonHost` borrows the same reference. Existing `BD0960` Source
publishes an actual tracked section at raw manager+10. A new mirror manager
cell or unrelated lock would not establish the observed operands.

The observed `BD0C30` edge has the concrete raw registration wrapper. It
validates current bounds before testing P, then appends a nonnull P through
the address of its actual argument word. It contains no lock, retain,
destruction or duplicate suppression. The existing raw-vector allocation and
returning Source CRT handler qualifications remain in force.

Current manager and registration CPP/header files match eight frozen source
and compiler-input copies across two verified manifests. Their previous
builds and archive proofs were not rerun. Current Source also supplies the
actual eight-byte guard layout, tracked-word access and direct Win32 service
calls. These are concrete reuse components, not a ready Application constructor.

The existing `destroy_native_singleton_guard_00411ee0` accepts an actual
eight-byte guard and real section pointer. Its availability does not establish
a constructor exception edge to it. Another Source caller's try/catch or
state-arming policy cannot be imported into this chain.

Current bounded Source search finds only existing `737970` comments/logging,
not the three actual constructor implementations or Application profile
bindings. The host still builds projected frame bookkeeping; its locale
integration records the absence of a genuine `E1AE90` owner. The retained Game
Application frame borrows genuine Application+14 and current-name cells; it
does not produce this whole owner or justify a detached replacement Game slot.

## Constructor exception and remaining ownership boundaries

This chain physically records `CC7240`, state zero before its base-table store,
the `CE37FC/S` guard, and state one after optional entry/increment. Its maps,
actions, exception release, base cleanup, publication retention/rollback and
fault behavior were not queried. Root's separate retirement `CC7260` review is
different evidence and does not close constructor exceptions.

The remaining concrete requirements are:

- A stable genuine 28-byte owner and shared actual `E1AE90` cell, with the
  established preimages and store order preserved through every consumer.
- Callable Source profiles for `D68BC4`, `D68BC8` and `CFEAB0` at their actual
  visibility points. Original address words and known slot-zero scalars alone
  do not supply complete callable tables.
- Explicit constructor composition using the existing canonical services,
  captured receiver/section, second manager and current publication operand.
- A separately established `CC7240` exception/fault contract or explicitly
  qualified Source failure policy, followed by complete owner lifetime and
  retirement integration.

All 209 fresh bytes and 63 instructions match the installed PE and the prior
producer capture. Current Source/target/prior pins and all canonical references
from the three reused audits were replayed, including bounded excerpts. Only
this document and its report changed. There was no Source, CMake, ledger or GPR
mutation, build, test, probe, native execution or new Original credit.
