# Dyn radix entry append: 00C37180

This packet reconstructs the complete normal body of `00C37180`, currently
named `DYN_physics_00c37180` in Ghidra. `append_object_00c37180` and the field
roles below are descriptive hypotheses. The implementation uses the observed
Win32 layout and explicit heap bindings; it does not supply a native register
ABI adapter, link registration, a recovered parent, or runtime activation.

The body is `00C37180..00C37202` inclusive, 131 bytes. The final instruction is
`RET 4` at `00C37200`; the last two bytes belong to that instruction. The prior
ledger record was `metadata_only_no_function_body_verified_oct24` in
`docs/CC13_DYN1.md`. Shared ledger replacement and build registration belong to
the integrator and are outside this packet's four-file lease.

## Evidence and original ABI

The checked target configuration is `C:/Users/sqz269/bsp.gpr`, project `bsp`,
program `/battlestationspacific.exe`, language `x86:LE:32:default`, image base
`00400000`. Read-only `bsp.py ghidra proto` calls verified the configured live
project/program before each cached pseudocode/listing batch. Evidence is in
`exports/bsp/functions/00c37180/{decompiled.c,assembly.txt}` and the caller's
corresponding `00c55d30` exports. No Ghidra annotations were changed.

ESI is the manager receiver; the object is one stack argument; the function
ends with `RET 4`. EBX and EDI are saved unconditionally, EBP only on the growth
path, and ESI is unchanged. The final EAX is the supplied object pointer, but a
semantic return value is not established. The explicit C++ interface returns
void and does not reproduce register/stack transport.

| Evidence | Recovered operation |
| --- | --- |
| `00C37182..00C37190` | Capture uint32 count at receiver+08h, form wrapping count+1, and grow only when new count is unsigned-greater than both old count and capacity at +0Ch. |
| `00C37192..00C3719E` | Compute wrapping uint32 allocation bytes as new count times 8, publish capacity first, then allocate. |
| `00C371AA..00C371D1` | Reload count while copying two dwords per old row; reload old storage at +04h each copied row; skip the copy only when that row's destination address equals zero. |
| `00C371D7..00C371E7` | Read current old storage, free it if nonzero, then publish the newly allocated pointer at receiver+04h. |
| `00C371EB..00C371FA` | Publish new count, write the object to old-index row+04h, then write the old count's low 16 bits to object+20h. |

The only direct external calls from this body are
`00C3719E CALL 00BF55BE` (operator new binding) and
`00C371DF CALL 00BF6989` (free binding). The callbacks are required providers,
not reconstructed vendor bodies or silent fallbacks.

## Preserved limits and uncertainty

The raw implementation uses uint32 address arithmetic and ordered volatile
word/short accesses, matching the repository's existing native Dyn storage
style. It intentionally does not substitute a C++ container or `memcpy`.
Growth is exact to the new count, rather than geometric. Count `FFFFFFFFh`
wraps to zero and bypasses allocation; the scaled allocation byte size also
wraps at 32 bits. There is no overflow rejection or clamp. The per-row null
destination test is preserved even when the cursor advances from zero to 8.
This is not an allocation-failure recovery path.

The append leaves the new entry's first dword untouched and preserves both
dwords of each copied old entry. It stores the row pointer before the object's
16-bit index and does not update indices in copied objects. Low-16 truncation
is preserved without assigning a signed interpretation. Valid writable
manager, entry and object storage is required for a successful completion.
No null-object guard is added. Heap exceptions may propagate after the capacity
write; native exception transport and invalid-memory fault equivalence are not
claimed.

The independent read-only caller check found that `00C55D30` copies ECX to ESI
at `00C55D34` and retains it for both calls to this helper. Thus the observed
receiver fields are manager fields, rather than an invented vector wrapper.
That parent allocates a 24h-byte object and also passes null to this helper on
its failed-allocation path. The helper still stores that pointer and attempts
the object+20h write; the source preserves that behavior.

## Validation and integration obligations

The report contains both direct leaf call sites and both parent call sites;
`verify_report_calls.py` checked all four with zero failures. Its Markdown
check also verified both direct leaf call claims with zero failures.
Source/assembly review covers allocation/free order,
copy ordering, unsigned wrap, untouched entry dword and low-16 index behavior.
No new tests, compiler runs, game runs or probe executables are part of this
worker packet. The primary integrator owns compilation, existing checks, CMake,
ledger/name changes, annotation review, integration and publication.

The only indexed caller is `Dyn_SAPRadixBroadPhaseManager_vslot0` at
`00C55D30..00C55D93` inclusive, 100 bytes, currently also metadata-only. Its
two helper call sites are `00C55D79` and `00C55D88`; its direct callee set is
`00BF681B` and `00C37180`. This packet does not recover that parent. Recovering
and connecting the parent, or proving an accepted dead chain, remains required
before a runtime activation claim for this leaf. No game/ABI/runtime validation
is claimed here.
