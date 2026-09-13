# Native hierarchy pool binding and entries

This packet supplies the four native entries at `B17AF0/B87A90/CD82D0/CE0ED0`,
146 original bytes, through existing `NativeMaterialParameterPool` algorithms.
The new `native_resource_hierarchy_pool.hpp/.cpp` adds an explicit canonical
companion binding for the application's actual hierarchy owner `109022C`.
It creates no native pool storage, copied state, free list, initialization guard,
callback registry or process startup wiring.

The [report](../reports/native_resource_hierarchy_pool.json) pins complete
live/installed-PE spans, original ABIs, all call sites, old names/comments and
the undefined startup function. The installed binary remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live batch used the target-verifying repository CLI for project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Ghidra was read-only.

| Native entry | Complete inclusive span | Bytes | Original ABI | Source coverage |
| --- | --- | ---: | --- | --- |
| Return slot `B17AF0` | `B17AF0-B17B57` | 104 | ECX initialized 38h pool; stack slot; RET 4 | Complete via existing return mechanism |
| Allocate `B87A90` | `B87A90-B87A99` | 10 | Ignore incoming ECX size; select 109022C; tail B185A0; EAX slot | Complete delegation |
| Startup `CD82D0` | `CD82D0-CD82E5` | 22 | No args; RET; EAX registration result | Complete delegation, raw native function pending definition |
| Shutdown `CE0ED0` | `CE0ED0-CE0ED9` | 10 | Select 109022C; tail B18470 | Complete delegation |

## Actual owner and binding

`109022C` is a distinct 38h pool. Its allocator element is at +0/+4/+8, actual
24-byte critical section at +0C=`1090238`, depth at +24=`1090250`, slab table at
+28=`1090254`, count/capacity at +2C/+30, and first nonfull slab at +34=`1090260`.
Its `D5E51C/B18500` profile and 88h slot shape match the parameter pool, but its
storage, lock, list element, table and slabs differ from material owner `F8D3E4`.

The application must construct a companion borrowing **that actual hierarchy
storage and the same `AllocatorListDomain` used by other native pools**. Then
call `bind_static_native_hierarchy_pool_0109022c` before startup. Binding remains
immutable thereafter. The companion, storage and shared list must survive the
real `atexit` callback, with every hierarchy payload already dead before pool
shutdown. Binding the material owner's companion does not satisfy this contract.
These identity/lifetime requirements are explicit preconditions, following the
existing `native_material_factory` binding pattern; no fallback instance or
runtime identity registry is added.

The existing pool source was unchanged from the accepted discovery baseline.
Its methods borrow supplied `storage_` and the shared allocator list. Their
descriptive `native_global=F8D3E4` constant does not redirect accesses there.
They supply the real constructor, allocation, return, trim and destructor
mechanics; this packet does not duplicate those algorithms or claim a new
all-aliasing/null-allocation domain.

## Entry schedules

Allocation loads the canonical companion and calls `allocate_slot_00b185a0`.
The original size input in ECX is overwritten by native `109022C`; the source
interface therefore takes no size argument. Existing allocation returns a raw
88h slot with hidden slab ID at +84. No 132-byte payload initialization occurs.

Startup calls `initialize_00b18340` first, then real
`std::atexit(&destroy_static_native_hierarchy_pool_00ce0ed0)`, returning its
integer result unchanged. The native calls are `CD82D5 -> B18340` and
`CD82DF -> BF6FF5`; `POP ECX` consumes the registration argument before RET.
Constructor failure never reaches registration. Registration failure does not
add rollback. The wrapper has no local EH frame; constructor cleanup belongs
to the existing provider. No startup hook calls this wrapper automatically.

Shutdown calls `destroy_00b18470` on the same canonical companion. Existing
destruction frees slabs/table, drains positive recursion, deletes the section
and unlinks the actual allocator element. It performs no payload destruction.
The source callback follows the existing factory's `noexcept` shutdown interface
within its supported initialized-storage domain.

`return_native_hierarchy_pool_slot_00b17af0` takes an explicit concrete pool
companion and delegates `return_slot_00b193fa_fragment`. Native B17AF0 also serves
material callers; the supplied companion preserves that owner selection instead
of forcing all returns through the hierarchy binding. Callers at `B191FC` and
`B19299` supply `F8D3E4`; `B88335` supplies `109022C`.

The existing return algorithm locks pool+0C before loading current slot+84,
indexes the current slab table, converts the signed byte delta into an 88h slot
index with division toward zero, pushes its WORD index at slab+4400h, increments
the current WORD count at +4500h and lowers firstFree+34 using unsigned comparison. It decrements
depth and releases the same section. No name cleanup, validation, slab reclaim
or record destructor is added. The name `hierarchy` describes this packet's
use of the shared entry, not an exclusive native type.

## Evidence and validation boundary

`CD82D0` still has no Ghidra function. Its complete 22 bytes match live memory
and the installed PE, and its pointer at `CE35B0` and callback target are retained
from accepted discovery. The report includes both actual address/native call
rows with `no_ghidra_function=true`. Current mechanical verification therefore
reports exactly two expected containing-function failures. The root integrator
must define `CD82D0-CD82E5` and rerun the unchanged report before integration.
The other six direct call rows are mechanically checkable; two IAT calls in
B17AF0 preserve their actual Enter/LeaveCriticalSection operands.

The strict MSVC Win32 Release build passed, including `/W4 /WX /fp:strict`.
Both existing CTests passed: `reconstructed_math` and `native_math_differential`.
Those math tests do not exercise these wrappers. No new capsule, fixture or
runtime execution is added for these delegating wrappers.
Current production objects, the unique archive, symbols/disassembly, compiler
commands and header/project closure are retained under `local/hierarchy-pool-bf/`.
The unique archive contains the exact current production wrapper object. Its
symbols reference the four existing pool methods and real `_atexit`; the current
provider object defines all four methods. The preserved build evidence includes
265 compiler read/input files and six compiler/tool binaries.
Current startup disassembly calls the provider before `_atexit`, then consumes
one argument and returns EAX. The source shutdown's `noexcept` boundary emits
compiler EH metadata, unlike the native tail jump; equivalence if an unsupported
provider exception escapes is not claimed.
This is source/listing and build validation, not a demonstrated application
binding, successful startup/shutdown sequence, native ABI replacement or game
validation. Original EH/runtime and invalid-memory equivalence remain unproven.

`B88320`, hierarchy fields and arrays, producer/append ownership, finite item
deletion, manager/cache operations and `B88430` runtime integration remain
separate packets. The older discovery and fields worktrees/evidence are preserved.

## BF integration correction, 2026-09-13

The isolated BF integration on 2026-09-13 defined and range-verified all eleven previously missing functions, including `CD82D0-CD82E5`, against the installed PE and live Ghidra bytes. The seven nondeleting leaves, token startup entry and two compiler exception handlers remain analysis-only. Evidence is retained in `reports/native_resource_lifetime_bf_function_definitions.json`. The local `B7D68C` call override was cleared and its nine-byte fall-through gap decoded; the stored `B7D640-B7D69E` body now covers all95 bytes with zero remaining call gaps. Existing full-function documentation was archived before and after repair. The repair changes analysis metadata, not the installed game. See `reports/native_resource_lifetime_bf_flow_repairs.json`.

The unchanged report now passes all8 direct-call rows with zero failures. Historical missing-function flags and worker-stage limitations above describe the retained original observations. New source comprises nine complete bodies (494 original bytes) across the three BF source packets; pool wrappers remain build-only. Combined candidate validation is recorded separately; this correction does not claim gameplay validation.
