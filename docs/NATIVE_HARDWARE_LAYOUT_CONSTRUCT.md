# Native hardware-layout construction

This packet reconstructs complete `B60790` stream creation and `B60CB0`
derived construction over actual native owner/stream storage. It composes the
native field operations from `NATIVE_HARDWARE_LAYOUT_FIELDS.md`, the existing
hardware owner cleanup, actual-header strings, diagnostic cleanup, and support
singleton services. It does not substitute the older high-level vertex-layout
conversion fragment for these two complete routines.

| Entry and complete native span | Original ABI | New C++ function |
| --- | --- | --- |
| `B60790..B60A00`, 271h / 625 bytes | ECX owner, stack stream list, RET4; no stable result | `create_native_hardware_layout_from_streams_00b60790` |
| `B60CB0..B60D00`, 51h / 81 bytes | ECX owner, stack stream list, EAX original owner, RET4 | `construct_native_hardware_layout_00b60cb0` |

`NativeHardwareLayoutConstructContext` borrows an existing
`NativeHardwareLayoutOwnerContext` and `NativeStringStorage`. It adds no
private renderer, cached device, support singleton, refcount, allocator state,
or operation callback. These are new MSVC Win32 C++ interfaces, not the
original binary calling conventions. Descriptive names remain hypotheses.

## Stream conversion and scratch storage

The input has four declaration pointer words at +00..+0C and a signed count
at +10. `B60790` tests the initial current count; zero or negative count skips
all stream work. Each reached input pointer is captured, passed to native
`B48A00` append/retain, then its current raw count+10 is tested. Each raw element
reloads current data+0C, adds the current 14h-byte offset, and reads offset as
a WORD; type, method, and usage as BYTEs; then usage again as a full DWORD.
The full usage indexes the shared usage-counter array. Its low BYTE becomes
the output usage index before incrementing the full counter. The emitted
element is two ordered DWORD stores, each using the current output count;
the count is incremented afterwards. Raw count is reloaded at every loop test.

Native frame offsets relative to ESP before the loop's additional ESI push
are +30..+64 for fourteen DWORD usage counters, +68..+E7 for sixteen eight-byte
packed output slots, and +E8 for the live output count. The source uses that
same contiguous relative layout: counters, 128-byte output, adjacent count.
Only counters and count are initialized. Unused packed bytes remain untouched.
After all streams, END is written as DWORDs `000000FF` and `00000011`, and the
count is incremented. This is a fixed scratch buffer, not an unbounded vector.

The source interface explicitly requires reached stream indices 0..3, valid
append targets and raw storage, full raw usage DWORDs 0..13, and at most fifteen
converted elements across all reached streams. END occupies the final
available slot. These are domain preconditions, not added runtime checks.
Element sixteen would write its first DWORD over the native count, then use
that overwritten count for the second store; later writes can corrupt the
exception frame. Such inputs are outside this reconstruction's source
contract. This bound comes from assembly, not a presumed D3D API limit.

After each stream's diagnostic lifetimes finish, the stream index increments
and current input count is reloaded. Consequently changes to later raw headers,
input count, or renderer globals at a real storage/service boundary remain
observable.

## Actual strings, current COM state, and cleanup

Every reached stream initializes an actual eight-byte temporary string header
and resizes it to twelve characters with preservation enabled through existing
`41DD40`. It captures pointer then length and copies `VertexFormat\0` from
immutable `D61BD0`, including the terminator. It then captures current
owner COM+40 into the first word of a twelve-byte borrowed diagnostic record,
arms state0, initializes its string at +04/+08, resizes it to the captured
temporary length, captures its pointer and length, and copies that many
characters. State1 is armed only after the second copy.

Actual `B3E730` is called with the shared published support slot and lifetime
domain. Native passes no diagnostic-record argument to this getter. On normal
return the state becomes0; diagnostic data is released using its captured
pointer and captured length+1. State becomes-1 before the temporary's normal
release, which likewise uses its captured pointer and length. A zero-byte
standard C++ memcpy is omitted under the existing native-string interface's
documented host boundary; no owned data is read or written by that omission.

Creation's handler is `CC1326`, FuncInfo `DFA0B8`, unwind map `DFA0A8`:
state1 action `CC131B` sends current diagnostic record `EBP-D4` to actual
`B3F4C0`; state0 action `CC1310` sends current temporary header `EBP-DC` to
actual `41DD20`. The cleanup paths therefore reread current headers, unlike
normal captured-pointer/length release. Armed cleanup objects preserve these
lifetimes without a catch/rethrow boundary that would alter exception search.
`NativeStringStorage::release` is the existing nonthrowing service boundary.

END's first word is stored before the global renderer load at `B6099C`.
The code captures current `F8D394`, writes END's second word and increments
count, then reads that renderer's current device+1A10, the device's current
COM table, and slot+158. It passes the actual `owner+40` output address.
`B60790` neither clears nor releases an existing COM field. HRESULT is ignored;
current owner stride is recomputed through `B47D60` after the COM call returns,
including failed HRESULTs. A COM exception skips that recomputation.

`B60CB0` first completes `B48C00`, arms base cleanup, clears owner+40, installs
`D62AF4`, and calls complete `B60790`. Its handler `CC1358`, FuncInfo `DFA118`,
map `DFA110`, and state0 action `CC1350` dispatch captured owner `EBP-10` to
actual base `B48960`. Failure does not call derived `B60700`, release COM+40,
or return the hardware pool slot. A COM implementation that writes output
then throws therefore leaves that output field after base cleanup. The source
guard matches native termination during search if base cleanup itself raises
a second C++ exception; that previously established base-cleanup behavior is
composed here, without a new double-failure test.

## Verification and integration boundary

The focused ignored fixture compares both complete original bodies with the
new source. It checks 22 spans / 1,077 bytes, including all 706 owned bytes,
both native FH3 code/map chains, the exact literal, native profile/type-table
bytes, and explicit dependency boundaries against live Ghidra and the installed
PE. Each live read verifies `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. The installed executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Execution rechecks the PE spans and uses a private relocated image with
protected code and maps. All postimages are checked against only the declared
relocations and dependency adapters; neither owned body is replaced.

Native field calls, base cleanup, string resize/destruction, and diagnostic
destruction adapt actual receivers into the existing production bodies.
The support observer delegates to an unchanged compiled production support
body. The existing string-storage interface delegates actual allocation/free
to `crt_string_storage`; native normal pool-return sites use that same storage.
A single observed CRT malloc import returns null at selected actual allocation
sites. The real string allocator or real shared allocator then raises
`bad_alloc`; the latter uses its actual CRT new-handler path. The COM fixture
implements the actual current vtable slot and controls HRESULT/output/throw.
These are fixture service boundaries, not new production operation callbacks.

Six comparisons matched 30,494 DWORDs in 386-word snapshots: maximum valid
fifteen-element conversion with stream/raw-count and renderer/device changes;
derived construction despite failed HRESULT; negative input count; second
string allocation failure; actual support allocation failure; and COM output
write followed by exception. Snapshots include actual owner/declarations/raw
records, stream list, renderer/device identities, packed written output and
adjacent scratch count, string bytes and allocation/free order, support/lock
state, and uncaught-exception count. Both original creation and constructor
FH3 maps were observed three times in search and three times in unwind.
No unused packed bytes are read or compared.

MSVC Win32 `/W4 /WX /O2 /Oy- /EHsc /fp:strict` compilation passed for the new
source, native field dependency, actual support provider, and fixture. Link-map
checks identify all production providers. All eight native seeds matched;
`scripts/build.ps1` and both existing CTests passed. No permanent tests were
added. Detailed artifact pins and adapters are recorded in
`reports/native_hardware_layout_construct_audit.json`.

Only the two owned source/interface files and this document/report are changed.
The integrator must register `src/native_hardware_layout_construct.cpp` in
`bsp_core`, update B60790/B60CB0 native evidence and reconstruction ledgers,
preserve existing comments/names as appropriate, save Ghidra, and refresh
exports. The worker makes no shared CMake, metadata, or Ghidra mutation.
This establishes bounded reconstruction, strict compilation and fixture
agreement, not original binary ABI, game, COM-driver, visual, invalid-stack,
or concurrent-mutation validation.
