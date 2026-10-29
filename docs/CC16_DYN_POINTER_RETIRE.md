# Dyn pointer-list removal and conditional destruction

This packet reconstructs complete `00C55DF0..00C55E9F`: **176 bytes, 72 native
instructions, 13 branches and four calls**. The descriptive public name is
`bsp::cc16_dyn_pointer_retire::remove_and_destroy_00c55df0`. It operates on
actual raw list storage and untyped pointer values. It does not declare a
native object class or assert that the removed object was constructed as a world.

The source directly calls the existing complete
`destroy_native_dyn_world_00c4dc60` implementation in
`src/native_game_physics_lifetime.cpp`, passing explicitly borrowed actual
memory, lifetime services, progress and body-construction binding. Its name is
retained as an existing descriptive hypothesis; it is not new type evidence.
Three mandatory allocator callbacks keep `BF55BE`, `BF6989`, and `BF65AC`
custody separate. There is no new default provider, returning destructor stub,
replacement allocator, fabricated object/context or implicit null body binding.

## Native ABI and dataflow

Read-only verifying `bsp.py ghidra` queries checked project `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, image base `00400000`
against `config/target.json` and `C:/Users/sqz269/bsp.gpr` before every batch.
Both contiguous instruction pages and all 176 live bytes were read. Current
live bytes equal the installed PE slice, SHA-256
`c22318b75d909c0ee124db6bc5d813d3149781f1bf9b4a4607b2e7b819e0d6c4`.

Native inputs are ESI = raw `{data,count,capacity}` at offsets 0/4/8, EDX =
the pointer-equality needle; there are no stack arguments and return is plain
RET. ESI is never changed. EBP and EDI are saved; EBX is saved only along the
allocation path, then restored before the paths join. There is no assigned
semantic return value. Incidental EAX/flags are not this public void API's
result contract.

The search captures data/end using 32-bit wrapping `data + count*4`, then scans
four-byte slots for equality. It does not validate success. At the shared exit
it reads `[end-4]`, captures `[cursor]` as the removed value in EBP, and overwrites
`[cursor]` with the captured last element. The order, including the read before
overwrite, remains literal native assembly. A missing needle still reaches the
end slot and an empty list still reads before its buffer. No not-found or empty
repair is added; ordinary valid removal requires a present pointer/nonempty list.

Count is reloaded after the overwrite; EDI receives `count-1` with uint32
wrapping. Unsigned `JBE` skips growth for ordinary positive counts. The empty
case underflows to `FFFFFFFFh` and can enter the original resize path:

1. Compare desired count with the current capacity, publish desired capacity
   **before** requesting wrapping `desired_count*4` bytes from `BF55BE`.
2. Retain the returned pointer. Copy using repeated live count and data reads;
   a zero computed destination suppresses that individual write. This is not
   an allocation-success guard and does not abort subsequent processing.
3. Reload current old data, conditionally call distinct storage free `BF6989`,
   then publish the retained new pointer. Allocator callbacks can affect the
   fields that native code subsequently reloads; the source does not cache them.
4. Reload count and zero-fill until desired count, reloading data each iteration
   and suppressing only a zero computed destination. All increments wrap.

After those paths, the source tests the originally captured removed value,
publishes desired count, and only then branches on that test. A null captured
value skips both destruction and object free. A nonnull value is pushed for
the real `C4DC60` body; a **separate** push follows for distinct `BF65AC` object
free. Object free is reached only if destruction returns normally. No rollback,
catch or conditional-success shortcut is introduced. Null allocation returns
follow the native arithmetic/write/free schedule; exception/fault transport is
qualified below.

## Real callee and genuine borrowed context

Fresh native entry/tail inspection confirms `C4DC60` loads its object from the
stack and ends with `RET4`. It does not take the object in EAX/ECX. The existing
source body clears bodies and attachments, destroys the scene, releases arrays,
bucket storage and pages in its established normal schedule. This packet reuses
that implementation, its native construction-context requirements and its
existing evidence; it does not claim new coverage of that callee.

`DynPointerRetireContext` requires all five constructor arguments:

- Actual outer allocation callbacks for `BF55BE`, `BF6989`, and `BF65AC`.
- The removed object's actual destruction memory service.
- Its actual `NativeGamePhysicsLifetimeCalls` service object.
- A caller-owned `NativeGamePhysicsLifetimeProgress`, updated by the real callee.
- An explicit actual `DynBodyCreationContext*` binding. A null pointer is allowed
  only when the existing callee's real attachment/virtual-dispatch policy permits
  it; this packet does not provide a null default or infer that policy.

The caller must establish that the captured pointer actually satisfies the real
callee's field/lifetime contract and supply the same allocator/pool/table
contexts used for construction. Context, callbacks, memory, services, progress
and any body context must outlive the synchronous removal. They are borrowed,
not stored in native data or retained by these bridges. No class cast is used.

The old plate's incoming attribution is wrong: current live `C5A8B0` ends at
`C5B175`. The indexed `C5B183` call is in separate undefined code. Read-only PE
decoding of `C5B180..C5B189` gives `PUSH ESI; MOV ESI,[EAX]; CALL C55DF0;
POP ESI; RET`. This establishes a pointer-to-list wrapper with unchanged EDX,
not an object class or construction origin. Ghidra currently has no instruction
at `C5B183`; no function/listing was created during this packet. Genuine caller
type/context binding remains open before activation.

## Private adaptation and object checks

The private naked kernel retains all 72 native instructions with address marks.
It adds one Context* stack argument, four pushes of that argument immediately
before the four native call sites, relocated bridge calls, and a final `RET4`.
No new scratch register or cached native field is introduced.

| Native site / target | Context location before inserted push | Shim cleanup |
| --- | --- | --- |
| `C55E2F / BF55BE` | `[ESP+14h]` after EBP/EDI/EBX and size pushes | `RET4` removes context; original caller still removes size |
| `C55E5F / BF6989` | `[ESP+14h]` after EBP/EDI/EBX and pointer pushes | `RET4` removes context; original caller still removes pointer |
| `C55E8F / C4DC60` | `[ESP+10h]` after EBP/EDI and object push | `RET8` removes context plus original callee-cleaned object |
| `C55E95 / BF65AC` | `[ESP+10h]` after EBP/EDI and separate object push | `RET4` removes context; original caller still removes pointer |

Each shim copies `(context,original_argument)` to an ordinary cdecl bridge and
removes its two copied argument words. The destructor bridge's relocation names
the real existing five-argument C++ implementation; its emitted pushes forward
the exact captured object and all four borrowed service/context values. The
allocator bridges call the three separate supplied function-pointer slots.
The public cdecl wrapper loads ESI/EDX, pushes the actual reference address, calls
the private kernel and restores its own ESI/EBP. None of the bridges/public
wrapper introduces FP/SSE arithmetic or conversion in the inspected object.

Only the allocation call has a live volatile result: EAX becomes the retained
new-data EBX. The bridge and shim preserve that returned pointer. Other return
values, volatile registers and call flags are not consumed by the kernel before
being reset or overwritten. EBP/EDI/ESI and conditional EBX are covered by the
actual cdecl callee contracts and native saves. The kernel itself has no x87/SSE
instructions, constants, absolute globals or external data dependencies.

MSVC Win32 `19.51.36244.0` object-only compilation passed with
`/std:c++17 /permissive- /EHsc /fp:strict /O2 /W4 /WX`. A read-only independent
COFF/Capstone comparison checks all 72 source-marked/listed instructions against
the fresh live/PE bytes and 76 emitted kernel instructions. All unadapted IDs,
operands, widths, registers, addressing and order match. The sole encoding
difference is a shorter identity `LEA ESP,[ESP]`; it changes neither flags nor
ESP. All 13 branch sites map to their 11 proper targets. All 72 listed
instructions are structurally reachable. A native stack-depth CFG audit verifies
every join, normal return, actual call-site context offset and callee cleanup.
All four native call rows pass the actual call verifier. Full hashes and local
artifact identities are in the report.

## Integration and evidence limits

This is complete instruction/storage and normal callback source closure with
direct real destructor composition. It remains unwired. Root must register the
source, perform the integrated Win32 build/link/checks, inspect final symbol/
object resolution, and establish genuine caller object/context custody.
Native/public/private ABIs differ. Native code/frame/fault addresses,
FH3/SEH/unwind and exception transport are not supplied by this public interface.
Allocation throws, destructor exceptions, fault recovery and gameplay were not
exercised. Partial mutation is not rolled back. Existing callee evidence is
reused, not presented as a new native differential run for this caller.

No tests or native/probe/game launches were added or run. No CMake, shared
ledgers, existing source/callers, Ghidra state, installation, saves/profile or
power/process settings were changed. The one new in-house body is separate from
all reused destructor and vendor dependency bodies; none earns duplicate credit.

## Primary integration checkpoint

The primary reviewed the complete native listing, all header/source and real
destructor forwarding. Source is registered once in bsp_core; common Win32
Release and both existing suites pass (12.61 seconds). Rechecking the final
object covers all72 native instructions/76 emitted,13 branches/11 targets,
four real call relocations and balanced context/shim stack paths. A manifested
link-only executable forces the actual entry and resolves real C4DC60 from
native_game_physics_lifetime.obj. It was never run; no dummy object/provider
or destructor stub was supplied. The entry remains absent from the game map.
Genuine removed-object compatibility, construction/context custody, original
ABI/exception transport and activation remain open.
