# Native device-class registry lifecycle

Packet `orch4_device_registry_d4` reconstructs the singleton getter and complete
destruction path used by the device-class factory at `00443090`. Names below are
descriptive hypotheses, except the compiler-generated scalar-deleting destructor
name already identified at `00441840`.

| Routine | Bytes | Original ABI |
| --- | ---: | --- |
| `00441780..00441831` | 178 | no inputs, `EAX` result, `RET` |
| `00441360..004413BD` | 94 | `ECX` owner, `RET` |
| `0043EBF0..0043EC00` | 17 | `ECX` owner, `RET` |
| `00441840..0044185D` | 30 | `ECX` owner, stacked flags, `EAX` captured owner, `RET 4` |

The source uses the existing raw singleton manager, raw registration wrappers,
CRT allocation boundary, guard destructor and device registry array. Its only
registry owner is the actual 10h-byte shape `{profile,data,count,capacity}`.
The process wrapper exposes one canonical `E17BF4` publication cell; the generic
entry accepts a stable reference to the same actual cell for composed runtimes.

## Getter `00441780`

The fast path captures `E17BF4` once and returns that captured value. On a miss,
the function obtains the shared manager through `00415350`, captures its current
raw `manager+10` section, writes a raw `CE37FC` guard, enters the section and then
increments the section's physical DWORD at `+18`. The section pointer remains
captured for the whole slow path.

After arming EH state zero, the function rechecks `E17BF4`. A continuing miss
allocates exactly 10h bytes. A nonnull allocation is initialized in this order:
`CE44DC` at `+0`, then zero at `+4`, `+8` and `+0C`. Only then is it published to
`E17BF4`. The getter calls `00415350` again, rereads `E17BF4`, and registers that
current pointer through `00BD0C30`. Publication therefore precedes registration.
A registration exception retains the publication and allocation, matching the
native schedule.

Normal release decrements the captured section's current `+18` DWORD before
`LeaveCriticalSection`. The slow return reloads publication after that release.
The state-zero unwind map at `D86410` invokes `C5F8D0`, which passes the raw guard
at `EBP-14h` to the complete `00411EE0` cleanup. There is no allocation-cleanup
state in this function; `operator_new` failure occurs before an owner exists.

## Destruction and publication reset

`00441360` passes owner `+4` and zero to the existing complete resize routine
`00440180`, frees the current data DWORD, then clears `E17BF4` and writes the
base profile `CE3818` to the owner. Resize-to-zero changes count only under valid
storage; the following free consumes the still-published data pointer. The body
does not clear the data/capacity fields and does not release the borrowed class
pointers.

The destructor's only EH state uses unwind map `D86324`. Its action thunk
`C5F850` loads the owner and jumps to `0043EBF0`. That helper performs only the
same ordered publication clear and base-profile store, so a propagated resize
exception still unpublishes and converts the owner to its base profile.

`00441840` calls the complete destructor, tests the low bit of the current flags
stack slot, optionally frees the captured owner, and returns that captured
address. There is no unregister operation. The raw manager already popped this
owner before scalar deletion during `00BD0400` drain.

`CE44DC` is a one-entry profile: its sole DWORD points to `00441840`; the bytes
at `CE44E0` begin an unrelated string. The canonical source manager dispatcher
now recognizes `CE44DC` and calls this deleter against the process `E17BF4` cell.
It adds no callback, binding, private manager, or alternate owning registry.

## Repaired listing boundaries

The initial live Ghidra definition of `00441360` stopped at `00441399`.
File-backed executable bytes established the continuation `0044139A..004413BD`,
including the publication clear, base-profile write and epilogue. The initial
flow-gap query reported no gap because the missing instructions lay outside the
old function body. `00441840` separately had a three-byte listing gap at
`00441855..00441857`; raw bytes decode it as `ADD ESP,4`.

The parent integrator applied both coordinated repairs under the packet lease
and refreshed exports. Current live readback reports `00441360..004413BD` with
27 instructions and `00441840..0044185D` with 11 instructions; both have zero
listing gaps. This worker kept Ghidra read-only.

## Evidence and limits

The live project was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Saved pseudocode, repaired live listings, raw PE
disassembly, the two unwind maps and the `CE44DC` profile were compared against
the source. The report records every direct call in the four reconstructed
bodies. A strict Win32 build and both existing CTests passed. The focused test
creates the process registry twice, proves one registration, drains it through
the canonical raw manager dispatcher, checks `E17BF4` clearing, then exercises
the retained-owner scalar path with a non-dereferenceable borrowed class word.

These are new MSVC Win32 source interfaces, not drop-in native ABI replacements.
Numeric profile DWORDs are retained as identity and never invoked as C++ vtables.
Source C++ cleanup preserves the observed publication/profile effects but does
not reproduce original FH3/SEH frame layout, arbitrary hardware-fault cleanup,
provider volatile-register identity, or gameplay validation.
