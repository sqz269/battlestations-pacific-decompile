# Native system parameter slots

Addresses: 00B40860, 00B9A030, 00B9A080, 00B9A0D0, 00B9A130.

This packet reconstructs five complete normal bodies: 340 bytes and 73 native
instructions. Names describe observed operations and are hypotheses, not
recovered symbols. The new C++ interfaces borrow the actual global storage,
actual raw receiver, current native argument words and current constant bytes.
They do not establish original-caller ABI compatibility or game validation.

| Routine | Inclusive body | Bytes / instructions | Original ABI | Coverage |
| --- | --- | --- | --- | --- |
| `publish_native_system_parameter_slot_00b40860` | B40860..B4087F | 32 / 11 | ECX DWORD index; EDX source; no stack arguments; RET at B4087F; no semantic result | complete |
| `set_native_system_parameter_slot0_00b9a030` | B9A030..B9A078 | 73 / 17 | ECX receiver; four DWORD argument slots; RET10h at B9A076 | complete |
| `set_native_system_parameter_slot2_00b9a080` | B9A080..B9A0C8 | 73 / 12 | ECX receiver; three DWORD argument slots; RET0Ch at B9A0C6 | complete |
| `set_native_system_parameter_slot3_00b9a0d0` | B9A0D0..B9A126 | 87 / 16 | ECX receiver; three DWORD argument slots; RET0Ch at B9A124 | complete |
| `set_native_system_parameter_slot1_00b9a130` | B9A130..B9A17A | 75 / 17 | ECX receiver; two DWORD argument slots; RET8 at B9A178 | complete |

## Storage and instruction order

The global domain is the sixteen consecutive words at 0108FC30. The existing
`SystemConstantBuilderEnvironment::parameters_0108fc30` already borrows this
domain; it is not a provider that constructs storage. This packet introduces
no duplicate owner or parameter-bank type. The caller must supply the actual
identities and readable/writable extents used by the original instructions.
Pointer values for these new C++ arguments must remain stable during a call;
pointed-to argument, constant, owner and publication bytes may overlap.

B40860 first performs FLD from source lane0. Only then does it shift the raw
DWORD index left four bits and add the bank base, retaining native 32-bit wrap.
Its four FLD/FSTP pairs are sequential. Each next source read observes the
preceding destination store. There is no range guard, snapshot, memcpy or SSE
substitution. x87 exceptions, NaN conversions and denormal handling belong to
the caller's current x87 environment; the implementation does not reset it.

| Setter | Actual record | Ordered operations before publication |
| --- | --- | --- |
| B9A030 | owner+3BC, slot0 | FLD argument0; MOVSS argument1; FDIV current CE47A0 double; store argument1 at +3C0, freshly read argument2 at +3C4, freshly read argument3 at +3C8; FSTP local float, FLD that float, FSTP +3BC |
| B9A080 | owner+5D0, slot2 | MOVSS current arguments0,1,2 into +5D0,+5D4,+5D8 in order; read current D7A24C bits and store +5DC |
| B9A0D0 | owner+5E0, slot3 | FLD argument0 then independently MOVSS its bits; FLD1 and FDIVRP form 1/argument0; ordered raw stores of current arguments0,1,2 into +5E0,+5E4,+5E8; read D7A24C and store +5EC; FSTP reciprocal to +5F0 |
| B9A130 | owner+3CC, slot1 | FLD argument0; MOVSS argument1; FDIV current CE47A0; argument1 to +3D0; XORPS positive zero to +3D4; current D7A24C bits to +3D8; FSTP local float, FLD that float, FSTP +3CC |

Each setter then calls the complete B40860 helper with its captured actual
record address. In particular, the reciprocal at owner+5F0 is a required
side effect outside slot3's published float4. Replacing the quotient roundtrip
with a wider result or combining raw argument reads changes observable behavior.
The inline assembly also keeps arithmetic and memory accesses in their native
order when an x87 operation faults. Unrestricted SEH recovery, private stack
aliasing, concurrency and instruction-pointer identity are outside this new ABI.

Live and installed-PE constant bytes were compared on 2026-09-13:

| Address | Bytes | Observed value |
| --- | --- | --- |
| CE47A0 | `00 00 00 00 00 40 8f 40` | double1000.0, bits408F400000000000 |
| D7A24C | `00 00 80 3f` | float1.0, bits3F800000 |

They are current borrowed memory operands, not compiled-in literals. The FDIV
reads precede owner stores; D7A24C reads follow the earlier raw owner stores.
This distinction matters if those identities overlap.

## Call and producer evidence

Every packet callee and stored body was read live. B40860 has no callees. Each
setter has only the helper call below. Full prototypes/body ranges verified
ownership of each numeric call site before recording it.

| Containing function | Call site | Native callee | Contract |
| --- | --- | --- | --- |
| B9A030 | B9A06E | B40860 | ECX0, EDX captured owner+3BC |
| B9A080 | B9A0C1 | B40860 | ECX2, EDX captured owner+5D0 |
| B9A0D0 | B9A11F | B40860 | ECX3, EDX captured owner+5E0 |
| B9A130 | B9A170 | B40860 | ECX1, EDX captured owner+3CC |
| 78C9B0 | 78CD92 | B9A030 | ECX EBX; four x87-spilled DWORD arguments from ESI+1B4,+1B8,+1C4,+1C8; callee RET10h |
| 78C9B0 | 78CE12 | B9A080 | ECX EBX; three x87-spilled DWORD arguments from ESI+1D8,+1DC,+1E0; callee RET0Ch |
| 78C9B0 | 78CCFF | B9A0D0 | ECX EBX; three x87-spilled DWORD arguments from ESI+38,+3C,+40; callee RET0Ch |
| 78C9B0 | 78CDAF | B9A130 | ECX EBX; two x87-spilled DWORD arguments from ESI+1BC,+1C0; callee RET8 |

Current direct xrefs identify 78C9B0 as the sole caller of each setter. Its
78CCE3 load captures EBX from EDI+A8 before these calls. The relevant caller
argument preparations and receiver provenance were read from its stored body;
they establish these slots, not the receiver's complete constructor or semantic
field names. These setters themselves are the producers of their raw offsets.
The whole 78C9B0 caller and the B46A70 consumer remain independently incomplete.

## Verification and limits

`config/target.json` pins C:/Users/sqz269/bsp.gpr and program
`/battlestationspacific.exe`, x86:LE:32:default, image base00400000. Every
`bsp.py ghidra` read/export constructs the verified BSP client before its batch.
All five full bodies were exported read-only; their byte spans and both constants
match the installed executable. Full span hashes and numeric call rows are in
`reports/native_system_parameter_slots.json`. No worker Ghidra annotations,
prototype edits, saves, CMake edits or repository tests were added.

Strict own-source MSVC Win32 compilation uses /O2 /W4 /WX /fp:strict. Its emitted
assembly was inspected for the four forward x87 publication pairs, both explicit
binary32 quotient roundtrips and the unspilled reciprocal before +5F0.
The baseline `scripts/build.ps1` and external fixture results are recorded in
the report. The baseline build excludes this unregistered worker source.

The focused external fixture lives in
`C:/Users/sqz269/bsp-az-system-parameter-slots/worker_capture.zip`. It preserves
the five unchanged original spans under one equal code relocation in a disposable
process, retaining all original relative helper calls and absolute memory operands.
The data operands use their original addresses; no instruction byte is edited. It compares
entire borrowed arenas, x87 exception flags/C1/TOP and the unchanged control
word. Only native private return/local stack slots are excluded. Inputs include
12 precision/rounding modes, eleven float bit patterns, six setter alias
layouts, changed current constants, helper overlaps and DWORD index wrap.

This bounds arithmetic and ordering evidence. It does not prove original caller
ABI, unrestricted unmasked exceptions or SEH unwinding, races, full initialization
of the native owner, the larger authored-world path, B46A70, or gameplay.

## AZ integration analysis refresh

The integrator saved and read back all 27 AZ original signatures and complete
normal-body ranges, and refreshed exports. CBBBF0 and CBBC10 are ten-byte
analysis-only EH handlers defined under leases and the write lock. Earlier
missing-function observations remain worker capture history. The batch adds
22 complete body records and extends five existing bodies with raw interfaces;
the two EH definitions add no normal-body count. Exact combined validation
follows separately from worker fixture evidence.

## AZ exact merged validation

The exact combined source commit `eedda791230482a4ac7ccd26a6d1f214b72bd6ba` passed the strict Win32
build, both existing tests and five current-library-only original-byte fixtures.
See `reports/native_system_sources_az_validation.json` for hashes, preserved
captures, case coverage and limits. Earlier pending statements describe worker
capture stages. Full rendering, native ABI and general concurrency remain open.
