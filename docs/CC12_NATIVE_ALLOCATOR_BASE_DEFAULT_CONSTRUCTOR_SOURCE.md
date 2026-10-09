# Native allocator base default constructor: Source candidate

This candidate replaces the private typed helper's volatile assignments with
a call to a raw MSVC Win32 adapter for the complete `00BF632F..00BF633F` body.
The adapter explicitly preserves the two DWORD read-modify-write clears,
their order before profile publication, the receiver result and final flags.
The accepted Original body is 17 bytes and five instructions.

The new Source is **unregistered and unbuilt** in this packet. Source-level
operation mapping and pinned evidence have been checked; actual emission,
Core membership, typed-consumer/EH behavior, normal build and admission remain
with the primary integrator. No Original ABI or gameplay claim is made.

Baseline: `cf257e747e130480f385233b5d4b2abcaebbb0ec`, containing the reviewed
default-constructor audit. Only the new header/source, the exact existing
helper migration, this document and the evidence report are changed.

## Raw interface and complete schedule

```cpp
void* __fastcall construct_native_allocator_base_default_00bf632f(
    void* actual_receiver,
    std::uint32_t unused_edx);
```

The public boundary is guarded for MSVC Win32; the definition is naked and
contains only the following five inline assembly operations. Pointer and
DWORD widths are asserted to be four bytes. No raw `noexcept` is declared.

| Original address | Bytes | Source assembly operation |
| --- | --- | --- |
| `00BF632F` | `8B C1` | `mov eax, ecx` |
| `00BF6331` | `83 60 04 00` | `and dword ptr [eax + 4], 0` |
| `00BF6335` | `83 60 08 00` | `and dword ptr [eax + 8], 0` |
| `00BF6339` | `C7 00 70 93 D6 00` | `mov dword ptr [eax], 00d69370h` |
| `00BF633F` | `C3` | `ret` |

Whole Original body: `8bc18360040083600800c7007093d600c3`.
SHA-256: `3aed2bc04e920f18daf703e4b76ea0d60eb59e4977578ab35b6403cd5fd6ca70`.
The Source statements map directly to every owned instruction, with no child
symbol or branch substitution. Actual compiler emission is still unverified.

Entry `ECX` holds the actual receiver `D`; the explicit second argument binds
an unused incoming DWORD to `EDX`. `EAX` becomes `D`, and both `ECX` and `EDX`
remain unchanged by the complete body. The raw result exposes `EAX=D`.
Nonvolatile registers are untouched. There are no explicit stack arguments,
pushes, local frame, child calls or x87 operations. Plain `RET` reads the
return address and advances `ESP` by four, without stack-argument cleanup.

The caller must supply actual leading 12-byte backing. The first `AND` reads
and writes the entire DWORD at `D+4` to zero; the second does the same at
`D+8`. Only afterward is `00D69370` written to `D+0`. The required access is
read/write at `+4` and `+8`, and write-only at `+0`. No previous profile value
is read and no message pointer is dereferenced, copied or freed.

The second `AND` leaves `CF=0`, `OF=0`, `ZF=1`, `SF=0`, `PF=1`, with `AF`
undefined. The profile `MOV` and `RET` preserve those flags; other flag bits
are unmodified. There are no additional guards, default memory reads, prior
free, field resets beyond the two observed clears, callbacks or local EH.
No `LOCK` or atomicity promise is added. Backing validity, lifetime and the
behavior of faults in the surrounding runtime remain caller contracts.

The raw profile is opaque numeric data. It does not create a callable table,
Native exception type, RTTI, static failure owner, slot, throw-info binding or
pointee identity. The descriptive Source API name remains provisional.

## Actual typed-owner migration

The existing private helper retains its `void` result and `noexcept` policy:

```cpp
void construct_base(NativeLegacyExceptionStorage& owner) noexcept {
    // Original constructor 00BF632F..00BF633F, 17 bytes. The actual typed
    // owner supplies leading 0Ch backing for both ordered DWORD RMW clears.
    (void)construct_native_allocator_base_default_00bf632f(&owner, 0u);
}
```

The actual `NativeLegacyExceptionStorage` owner is 40 bytes with the relevant
fields at `+0`, `+4` and `+8`, followed by its member at `+0Ch`. Existing
Win32 size/offset assertions establish this Source layout. The helper passes
that actual owner's address directly and discards the raw receiver result.
No raw 12-byte backing is cast to the larger type.

The owner file differs from the selected baseline only by the new include
and this exact helper replacement. The cleanup/copy delegates and all other
legacy code remain identical. The old endpoint comment `00BF6340` is corrected
to the actual final `RET` at `00BF633F`; no adjacent byte is read.

Calling a raw declaration without `noexcept` from the retained typed helper
can affect current compiler EH/terminate metadata. That effect requires the
primary's complete emitted consumer/EH review. No generated behavior is
assumed here, and existing Source policy does not establish Original EH parity.

## Evidence versions and build boundary

The accepted read-only report retains fresh live metadata, all five live
instructions, exact live bytes and a matching original-PE span. This packet
rechecks only that owned 17-byte PE span and makes no new Ghidra query or
mutation. There are no Native caller, child, handler, adjacent-body, static
owner, slot, profile, string or RTTI body/data queries.

All 179 inherited references match their accepted current-baseline hashes
before this migration: ten direct inputs plus 169 constructor/copy/cleanup
receipt references. Three references point to the one intentionally changed
owner file. Their accepted-baseline and candidate hashes remain separate.

All 169 receipt references also match their individual historical commits.
At this packet's baseline, 165 match currently and four older references
differ: three CMake references and one owner reference. After the intentional
owner edit, 164 match those historical hashes and five differ. All 54 latest
cleanup references match the baseline; the candidate changes only its owner
reference. These separate version domains are retained in the report.

The earlier 24-, 25-, 88- and 22-byte admissions remain separate receipts;
none proves or adds credit for this 17-byte candidate. The latest cleanup
receipt's reviewed Source 23-byte adapter, current UCRT import and typed EH
effects remain qualified context. Its builds, artifacts and execution are
not replayed by this packet.

The new file is absent from explicit CMake registration, and there is no file
glob that registers it. The existing owner is already registered. The primary
must register the adapter before the required normal `scripts/build.ps1`
build, review the complete emitted leaf and actual typed-consumer/EH graph,
then decide ledger/Ghidra admission. No worker build, test or probe was run.
Original placement, caller/runtime/exception compatibility, startup and
gameplay remain unproved.
