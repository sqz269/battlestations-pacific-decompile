# CC11 actual unit-group member searches

The source in `src/native_unit_group_lookup.cpp` reconstructs two complete
native functions over actual borrowed group storage:

| Function | Complete range | Bytes | Instructions | Result |
| --- | --- | ---: | ---: | --- |
| Member index | `0070D030..0070D058` | 41 | 16 | Full EAX index, or `FFFFFFFF` |
| Member record | `0070D080..0070D0B5` | 54 | 23 | Full EAX actual record pointer, or null |

Both originals have zero calls, globals and relocations. Native ECX is the
actual group pointer, the stack argument is an actual entity pointer, and both
functions end with RET4. These new C++ interfaces do not replace that native
class or register ABI.

## Complete storage behavior

Each function captures the signed DWORD at group `+4F8h` once. If it is
nonpositive, no record is read. Otherwise it compares the actual entity DWORD
at `group+18h + index*34h` with the query pointer, in increasing index order.
The first match wins. A null query can match a null slot; there is no native
null-query rejection, extra bound, fallback, handle conversion or callback.

The index function returns the first matching index or -1. The record function
returns the matching address inside that same borrowed receiver or null. It
does not copy or create a record. Neither function writes to the group or entity.
The domain is valid, aligned, full 508h-byte Win32 group backing. A positive
count describes readable records within the 24-record capacity. This domain
requirement is not an additional runtime guard.

Native register behavior differs between the two bodies: `0070D030` advances
ECX to the matching record, or to the end on a miss; it leaves ECX at the group
when count is nonpositive. `0070D080` preserves ECX at the group. Its caller at
`0070D2A6` immediately uses that preserved register. The fixture records and
checks these original postconditions, while the source remains a new C++ ABI.

## Actual callers and existing source

Pinned caller ranges establish real consumers separately from the leaves'
zero call counts:

- `00536086` calls the index search for list selection.
- `0053ACD0` calls the index search, then `0053ACDC` uses the index with the
  record accessor `0070D070`.
- `0070D2A1` calls the record search from formation station calculation.
- `0077FB80` calls the record search on the group reached through actual
  unit `+284h`; the following formation-order code uses that returned record.

The existing `GameUnitsHost::set_formation_member_offset_0070d080` uses hosted
indices, `member+1` handles, vector storage and added guards. It is a semantic
consumer and remains unchanged. This packet introduces the actual-storage
lookup boundary without claiming those larger callers are reconstructed.

Three caller rows pass the live function-scoped verifier. `0053ACD0` is outside
Ghidra's current function ownership, so the original four-row verification
reports one failure. That attempt is retained. The complete 18-byte range
`0053ACCF..0053ACE0` separately matches live memory and the frozen PE; decoding
proves its `CALL 0070D030`. This is a raw caller witness, not a repaired Ghidra
function or a fourth successful function-scoped verifier row.

## Focused original/source evidence

`reports/cc11_unit_group_lookup.json` pins the evidence under the unique ignored
directory `local/cc11_unit_group_lookup_20261007_a`. Ghidra access stayed
read-only against `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

One focused fixture uses one aligned, complete 508h-byte borrowed group with
two 16-byte guards and actual pointer identities into four borrowed entity
prefixes. Records include a duplicate identity, null holes, an interior member
and a member at index 23. Seven queries cover first match despite the duplicate,
null-hole match, interior, last, absent, zero count and negative count. Unread
group bytes remain canaries; no constructor or publisher is simulated.

Original and source calls share that same group, so pointer comparisons prove
the exact returned address, not merely a translated offset. All 1,288 group
bytes, both guards and every entity-prefix byte remain unchanged after each
query. The fixture passes **49 checks**, including **28 full-width result
comparisons** across the seven queries. The complete original 41-byte and
54-byte bodies retain all 95 bytes before and after execution, with no patches,
relocations or dependency bridges.

The fresh standalone MSVC Win32 build uses current source and fixture as its
two translation units, with `/O2 /W4 /WX /fp:strict /showIncludes` and an
embedded `asInvoker` manifest. Four actual repository/fixture inputs are frozen;
actual include tracing checks them and pins 173 host headers. Six searched
toolchain libraries are recorded. No BSP library is linked, and no prior
fixture is replayed. The original installed executable, its frozen copy, eight
previous worker reports and their 860 artifacts remain unchanged.

The compiler, linker and environment-script files are also hash-pinned by
resolving the same `vcvarsall.bat x86` environment after the build. Their toolset
root matches the libraries in the actual build log; this is an environment/file
receipt rather than an instrumented process-load trace.

Whole Source COFF review covers both functions: 57 bytes/25 instructions for
the index search and 56 bytes/25 instructions for the record search. Each
captures count at `+08`, takes its signed nonpositive branch at `+13`, and first
reads a record at `+20`. The loops compare the actual DWORD, stop on the first
match, advance by `34h`, and compare their index against the captured count.
The miss paths return full -1 or zero; the matching paths return the index or
actual record address. There are no calls or relocations. The complete entry
adapter is 36 bytes/15 instructions and records original EAX and ECX.

## Qualification

Both complete searches are reconstructed, strict-Win32-build-tested and
focused-original-fixture-tested over borrowed storage. This does not establish
the original class/register ABI, full unit construction, group publication,
observer or profile lifecycle, formation/settings closure, dispatcher
integration, concurrent mutation behavior, failure/unwinding behavior or game
validation. The primary owns shared metadata, registration and project builds.
