# Actual DWORD-vector header range destruction

| Native entry | Preserved library identity | Coverage | Source interface |
| --- | --- | --- | --- |
| `004D49B0` | `STL_inst_004d49b0` | Complete, 54 bytes / 25 instructions | `destroy_native_dword_vector_header_range_004d49b0`, fastcall, four formals |
| `004D9E40` | `STL_TagDispatch_004d9e40` | Complete, 22 bytes / 7 instructions | `destroy_native_dword_vector_header_range_dispatch_004d9e40`, stdcall, two formals |

The new descriptive Source names are hypotheses. Both correct library identities,
the existing input-configuration adapter, and earlier narrower interfaces remain
unchanged. This packet does not complete a containing vector algorithm.

The core captures EDX last before ECX first, compares the two endpoints, and skips
all header accesses when equal. For every reached 16-byte header, it captures
DWORD+4 once and frees that captured pointer only when nonzero. After either
returning path it unconditionally clears DWORD+4, +8, and +0Ch in that order,
advances by 10h, and compares against the original captured end. There is no
null-header skip, direct opaque+0 access, element traversal, snapshot or rollback.
Earlier visits remain visible to later visits.

Native first/last occupy ECX/EDX, followed by two ignored physical stack words;
the core returns with RET8. The Source preserves those argument slots and uses
32-bit uintptr arithmetic and volatile DWORD accesses. Its direct callee is the
accepted `singleton_lifetime_free`, which uses the current CRT. No header hook,
callback, provider substitution, typed count conversion or fake no-op is added.

The native dispatcher has first/last at entry ESP+4/+8 and also returns RET8.
It reads last into EAX, re-reads last into EDX, pushes the duplicate last and
ambient ECX as dead child metadata, then reads first into ECX after those pushes.
The new stdcall interface defines the ignored child words as zero. This preserves
effects under the explicit private argument-frame qualification; it does not
invent a meaningful owner or ambient register input.

The Source interfaces return void. On an empty core path native EAX retains its
incoming bits; on an empty dispatcher path it retains last. A nonempty path leaves
the final captured null backing or incidental void-free residue. These bits are
not a semantic pointer result. Both complete bodies contain no local EH setup,
catch, action, state table or x87 operation.

The exact normal transfers are:

```text
004D49C8 CALL 00BF65AC
004D9E4E CALL 004D49B0
```

Fresh typed metadata verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 76 owned PE/live bytes and all 32 current listing
starts agree with complete offline Capstone decoding. The frozen readiness receipt
also compares the accepted raw tidy services and their actual free targets.
The accepted copy constructor establishes the 16-byte header's +4/+8/+0Ch storage
fields; no element type or safe containing object is inferred from that layout.

The initial `006982B0` destruction hypothesis was wrong: its live listing has no
function or instruction there, and its 88 raw bytes decode as an unlisted erase
range body. Its cleanup target led to these actual entries. Complete current
`00698380` has only its accepted backward-swap target; the stale graph edge is not
evidence of a destruction dependency. Neither unlisted body is promoted, repaired
or implemented here. String's distinct conflict cleanup entries remain disjoint.

Reached headers must remain valid and writable through free and all three stores.
Every nonzero backing must belong to the admitted current CRT. The native
wrapping +10h/equality traversal must terminate through valid storage. No new
alignment, bounds, null or ownership guard is introduced. Original heap/CRT,
private frame aliases, FH3/SEH/fault behavior, incidental registers/flags, mangling,
drop-in binary ABI and game/runtime equivalence remain unproved. Caller EH scopes
remain caller-owned.

The checked FIFO CMake turn fast-forwarded accepted Main, appended this source
only, and released the registration lease before building. The required
`./scripts/build.ps1` passed all three existing suites in 7.99 seconds. The existing
duplicate `spawn_request_id_matches` linker warning remains outside this packet. No new test
was added; these suites do not specifically exercise the new raw interfaces.

The actual CMake object contains 131 code bytes / 47 decoded instructions. Its
fastcall core has 65 bytes / 22 instructions and its stdcall dispatcher has
66 bytes / 25 instructions; both end in RET8. The compiler inlines the Source
core in the dispatcher. Both bodies have the same 44-byte cleanup loop and the
same exact current-free symbol relocation, conditional free, ordered stores and
forward equality. The dispatcher uses its own balanced EBP frame and reads the
two stack arguments before the initial comparison. This Source frame/read order
is covered by the stated private-frame qualification.

An explicitly authorized object-only optimized Win32 compile independently
produced 130 bytes / 44 instructions. Its entire core and all loop bytes/relocations
match the CMake object; its dispatcher frame differs. Ten focused checks pass on
the actual CMake object. The two report transfers and two Markdown transfer claims
pass verification. Primary frozen-pin acceptance remains pending. No Ghidra write,
push, original-ABI fixture or game validation is claimed.
