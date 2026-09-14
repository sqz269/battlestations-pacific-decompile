# Native renderer pointer-array cleanup

The seven entries operate on an actual 0Ch header: current data +00, signed
count +04, signed capacity +08. Pointer cells are four bytes. No routine accesses,
releases, or owns any pointee. Query/vertex/index names describe observed callers;
they do not imply a reconstructed pointee layout.

| Native entry | Complete inclusive range | Operation/dependency |
| --- | --- | --- |
| B22530 | B22530..B2258E, 95 bytes | Query reserve; reuse actual B22D10 provider |
| B22CC0 | B22CC0..B22D0F, 80 bytes | Query resize; B22530 |
| B25940 | B25940..B2598F, 80 bytes | Vertex resize; existing actual B22D10 |
| B259D0 | B259D0..B25A1F, 80 bytes | Index resize; existing actual B22D70 |
| B27F50 | B27F50..B27F66, 23 bytes | Query resize0, then free current backing |
| B29B20 | B29B20..B29B36, 23 bytes | Vertex resize0, then free current backing |
| B29B40 | B29B40..B29B56, 23 bytes | Index resize0, then free current backing |

Reserve/resize ABI is ECX=header, one stack word, RET4. New fastcall adapters use
an explicit unused EDX parameter. Destructors use ECX=header and plain RET.
These remain source interfaces with a new CRT/private-frame exception boundary.

The complete B22530 body is byte-identical to the existing B22D10 and B22D70
bodies after masking only the two CALL rel32 operands. Both targets are the same
allocator and free. Its new named entry delegates to the substantive existing
`reserve_native_renderer_pointer_array_00b22d10`; no reserve implementation was
copied and no projected array was substituted. All three reserve spans were
freshly verified live against disk. The three resize bodies likewise differ
only in their reserve CALL operand; the destructors differ only in two CALLs.

Reserve clamps the request to at least one, compares signed current capacity,
allocates the wrapping request*4 byte size, and copies current count DWORDs while
reloading current source base/count. It preserves the per-destination null check,
frees current old backing, and only then publishes replacement data and capacity.
Count is unchanged. Allocation failure leaves this continuation unexecuted;
earlier allocation-handler effects are retained without added rollback.

Resize compares the signed request with current capacity before calling reserve.
After a returning reserve, it captures current count as its initialization index.
Each new cell address uses **current data + wrapping index*4**, skips a null
destination, and otherwise writes zero. The loop increments the captured index;
it does not publish count for each initialized cell. It then compares requested
count with current count and repeatedly decrements the current count if needed,
finally publishing the requested count. Removed cell bits remain untouched.
A negative initial count can therefore initialize cells before the nominal base;
valid reachable storage remains the caller's obligation. It is not clamped away.

Each destructor calls its actual resize with zero. Only after resize returns does
it load and free current data. It does not clear the stale pointer, capacity, or
header, and does not free pointees. A returning negative-capacity reserve can
publish new backing which is immediately freed by the destructor. A resize
exception bypasses the final free; no catch or cleanup rollback is added.

Read-only parent actions CBDF1E/CBDF2C/CBDF3A load the owner from EBP-14h and
adjust by +19A0/+19B0/+19C4 before tail-jumping to these destructors. Additional
actions CBDF80/CBDF8E use +1AAC/+1AB8 with B29B20/B29B40. Parent state assignment
and full renderer reconstruction remain integrator-owned.

The B22530 listing reaches B2258E through its early-return branch, but the free
call leaves a nine-byte gap B22581..B22589: stack cleanup, pointer/capacity
publication and POP EBX. Required returning-call override: B2257C. The destructor overrides
are B27F5D, B29B2D and B29B4D; saved ends B27F61/B29B31/B29B51 omit stack cleanup,
ESI restore and RET. Full live/PE ranges above include every tail byte. The three
resizes are already complete. The worker made no Ghidra mutation; the integrator
owns overrides, names, old-value evidence, save and refreshed exports after release.

Validation: eight native math seed ranges before configure, strict Win32 build,
two CTests, and one original/source fixture. The fixture executes all seven
complete original bodies with only CALL operands rebound. Query resize/destructor
use the original owned child bodies; vertex/index reserve leaves use the same
substantive existing source providers. The allocator/free boundary is the shared
lifetime allocation domain. Twenty-four comparisons cover reserve copy/clamp/no
growth, resize growth/shrink/signed negative indexing/null destination, and
destructor current allocation/stale-header postimages. No original EXE address
is invoked. Allocation failure is supported by static/source ordering, without
fault injection; original CRT/SEH identity and game behavior remain unvalidated.

The immutable local archive records source, built libraries, executable/object,
fixture/reference, tools, and logs. Runtime DLL evidence is separate: the running
x86 fixture observes loaded modules and resolves their file paths through handles
opened in that same process. Machine identity is measured from the loaded image;
hashes are subsequently measured from disk files and their archive copies, not
from mapped image memory. Observed tool executables do not constitute every
toolchain dependency.

## Integrated validation at 87dad090

Seven complete original bodies versus integrated source: 24 reserve/resize/destructor comparisons. Allocation failure remains static evidence.

The exact integrated source at `87dad0906ab21f6498d68eebacd8995ea86ddead` passed the strict MSVC Win32 build and both CTests. The existing focused fixture was relinked against that library with `/MD`, `/W4 /WX`, `/sourceDependencies` and an embedded manifest. Runtime module paths were measured inside the fixture; the capability capture occurs while its real Direct3D9 object remains alive. The packet passed 11 numeric direct-call checks.

The combined checkpoint retains 3413 immutable artifacts at `local/checkpoints/87dad090/native-renderer-lifetime-followups/validation.json` (SHA-256 `930898fb5129ef943689981be1fcdd48913c18e9154efa35665c19f74398bfc3`). It records 13 new entry bodies (713 native bytes), the separate capability ownership overload, 14 saved/read-back annotations, seven completed returning tails, 61 passing call rows and 45 live/PE spans totaling 18,479 bytes including reused/evidence spans. The B22530 RET4 starts at B2258C and ends at B2258E; stored-body ends and return-instruction addresses are recorded separately. Worker evidence remains preserved. Full renderer lifetime, application adoption, original exception identity and gameplay remain incomplete.
