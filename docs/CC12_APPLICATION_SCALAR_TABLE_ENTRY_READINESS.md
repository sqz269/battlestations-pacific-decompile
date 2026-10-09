# Actual Application scalar table entries

This read-only audit establishes the first DWORD of each observed Application
table and the complete three scalar-deletion entry bodies. It supplies no
callable Source table or new reconstruction credit. The ordinary destructor
dependencies remain open.

| Observed table identity | Slot zero | Complete body | Ordinary child |
| --- | --- | --- | --- |
| `00CFEAB0` | `00737E00` | 30 bytes, 11 instructions | `007379A0` |
| `00D68BC4` | `00BEA950` | 30 bytes, 11 instructions | `00BEA8B0` |
| `00D68BC8` | `00BEA9A0` | 36 bytes, 12 instructions | `00BEA8B0` |

The [report](../reports/cc12_application_scalar_table_entry_readiness.json)
retains all 108 freshly captured bytes: 96 code bytes and three table DWORDs.
They match the current installed PE. Table width, later slots and descendants
are outside this packet. The adjacent `D68BC4` and `D68BC8` cells are separately
observed table identities; their adjacency does not establish a two-slot table.

## Complete ordinary entry schedule

All three entries save ESI and capture the actual incoming ECX receiver in ESI.
`BEA9A0` then writes `D68BC8` to the actual receiver's first DWORD before calling
`BEA8B0` directly. It does not call the `BEA990` tail wrapper. The other two
entries make their ordinary child calls without a preceding local table write.

Only after the child returns does each entry test bit zero of the caller's
argument byte at `[ESP+8]`. If set, it pushes the captured ESI pointer, calls
the actual `BF65AC` deallocation entry, then executes `ADD ESP,4`. If clear,
it skips that entire free sequence. Each then copies the captured pointer to
EAX, restores ESI and executes `RET4`. Neither upper argument bits nor a
second flag govern this local schedule. The argument is read after the child;
this audit does not replace that read with an earlier cached C++ Boolean.

The returned captured address is not a live-ownership assertion. On the free
path it is returned without dereferencing the object afterward. There are no
local null checks, refcount operations, publication reads, manager lookups,
extra frees, catches or rollback operations. Child behavior and inherited
exceptional behavior remain external.

## Listing qualification

The Ghidra body ranges cover all 96 code bytes, but its current instruction
listing omits the three `ADD ESP,4` instructions after the free calls. Those
are present in the raw bytes and installed image at `737E15`, `BEA965` and
`BEA9BB`, encoded `83 C4 04`. A complete independent x86 decode yields
11/11/12 instructions. The current live listing yields 10/10/11. This packet
records that nine-byte listing gap; it does not repair saved analysis or infer
stack cleanup from an incomplete listing. Existing placeholder prototypes do
not override the physical ECX input and `RET4` argument cleanup.

## Current Source and ownership boundary

The observed startup Application is the same WinMain stack allocation through
construction, initialization, loop, shutdown and ordinary destruction. A
Source scalar entry therefore cannot automatically free every Application;
the actual caller flags and allocation domain matter. The normal outer shell
at `7379A0` retires its interior vector before forwarding to `BEA990`, whose
two-instruction body writes the base profile and tail-forwards to `BEA8B0`.
That prior evidence is retained, rather than expanded here.

`native_singleton_destruction.cpp` currently dispatches recovered profiles but
has no switch case for `CFEAB0`, `D68BC4` or `D68BC8`; unqualified profiles reach
its existing `logic_error` boundary. A bounded current address search likewise
finds no concrete Source implementation of the three scalar entries. Existing
raw manager registration/removal and allocation services do not supply the
missing Application owner, ordinary destruction contract or callable table.

Original numeric table words cannot become callable Source tables by copying
them, and a projected host cannot substitute for the real receiver. The next
bounded prerequisite is the complete ordinary `BEA8B0` retirement body, followed
by the actual outer vector retirement and qualified common Application owner.
No wrapper or owner implementation is admitted by this audit.

## Validation and limits

Live queries verified project `bsp`, program `/battlestationspacific.exe` and
unchanged function count 64729. Raw code, table cells, installed-image identity,
complete independent instruction boundaries, current Source pins and prior
read-only evidence were checked. No Source, CMake, ledger, Ghidra or listing
mutation, build, test, probe or new Original credit occurred. Static bytes and
entry contracts establish neither startup execution nor gameplay validation.
