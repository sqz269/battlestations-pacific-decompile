# Renderer record guard (R53)

The debug-record producer uses the actual `0108D5A0` singleton: an eight-byte
owner with profile `00D5E60C` and a tracked critical section at `+4`. These four
new bodies use the existing raw singleton manager and allocation/lock providers.
`NativeRendererRecordGuardContext` borrows the same actual `01090AA0` and
`0108D5A0` cells; it creates no second manager or semantic owner projection.

| Native entry | Bytes | Recovered behavior |
| --- | ---: | --- |
| `00B22360` | 17 | Unconditionally clear current publication, then stamp `CE3818`. |
| `00B23750` | 69 | Stamp `D5E60C`; create the genuine `BD1860` section. Constructor unwind runs `B22360`. |
| `00B25BE0` | 189 | Capture hot publication; otherwise capture first manager section, lock, recheck, allocate/construct, publish, resolve current manager, reload publication and register. Unlock captured section; reload cold return. |
| `00B26130` | 55 | Stamp derived profile, release actual `+4`, test flags bit0, clear current publication, stamp base profile, optionally free captured owner. Return that address. |

The scalar destructor does **not** unregister. The raw manager's finite source
dispatch now recognizes `D5E60C` when given this same guard context, pops before
deletion and calls `B26130`. Existing binding offsets remain unchanged; the new
pointer is appended at 116 and the source binding structure is now 120 bytes.
No application or startup binding is enabled by this packet.

## Exception evidence and lifetime

Fresh live Ghidra/PE comparison verifies the gate's `DF5788` FuncInfo and
`DF5778` unwind map. State1 frees the captured allocation via `CBD0C8` before
state0 releases the captured manager section via `CBD0C0 -> 411EE0`.
Constructor `DF54A8` / `DF54A0` state0 invokes `CBCEB0 -> B22360`.
State0 remains armed through the normal leave operation. The source preserves
this schedule for C++ exceptions and terminates on a secondary cleanup exception.

Allocator failure has no returned allocation to free. Constructor failure clears
publication, stamps the base, frees the captured allocation and unlocks.
Registration failure preserves the newly published owner and only unlocks; no
rollback, unregister or repair is invented. Destruction clears the **current**
publication even when it differs from the owner being destroyed.

The prior R52 listing repair restored `B2615E: ADD ESP,4` after an erroneous
call-return flow override at `B26159`; the complete 55-byte scalar was used here.
Its existing library name is retained. All names describe recovered behavior,
not original symbols.

## Validation and limits

- Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests.
- Ignored focused probe copies the full original 17-byte base and 55-byte scalar.
  Only the actual publication operand and scalar's two genuine release/free
  calls are relocated. Whole eight-byte owner state is compared for flags0/2;
  flags1 checks captured return/publication without reading freed storage.
  Both lanes use actual Windows sections acquired twice and tracked depth2.
- The same probe exercises source construction, a hot return with inaccessible
  manager sentinel, genuine cold manager creation/registration, repeat hot
  lookup without duplicate registration, and actual finite-dispatch manager drain.
- Native spans, direct calls, exception maps, strict build products, compiler
  inputs, probe inputs/outputs, loaded physical I386 modules and toolchain are
  sealed with hashes in the report's local evidence archive.

The full original gate and constructor are not executed. Allocation/registration
exceptions are not injected. Source C++ EH is not proof of original FH3,
private-frame/register ABI or hardware-fault compatibility. These new context
interfaces are not drop-in binary replacements. Active renderer, application,
visual and gameplay behavior remain unvalidated.
