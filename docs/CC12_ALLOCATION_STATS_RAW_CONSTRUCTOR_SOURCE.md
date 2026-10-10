# Raw allocation-statistics constructors

The standalone Source APIs now implement the accepted ordinary schedules of
base `00BE2750` and derived `00BE2900` on the caller's actual writable 12-byte
receiver. They borrow the actual manager `01090AA0` and stats `0109CEFC`
publication cells and call the existing genuine manager getter, registration,
guard cleanup, generic-base reset and Win32 section services. The receiver's
lifetime and subsequent deletion remain the caller's responsibility.

The change is limited to the new header/CPP and this document/report. Root
owns CMake registration, selected-input/archive registration, normal Win32
build/tests, ledgers and annotations. This worker's fresh compilation is an
isolated object build of the new CPP and four existing provider CPPs; the
candidate is not yet in a normal CMake target. The existing `platform_window`
scalar constructor and stack-local startup projection remain unchanged.

## Ordered Source contract

The base constructs its private Source base-cleanup guard before storing
profile `00D685E0`. It resolves the actual manager through `00415350`, captures
that first manager's section at `+10h`, and forms the exact eight-byte guard
with profile `00CE37FC` and that section pointer. If nonnull, it calls actual
`EnterCriticalSection` and increments the section's actual DWORD at `+18h`.
Only then does it arm guard cleanup and publish the original receiver.

It calls `00415350` a second time. After that call returns, it reloads the
CURRENT stats publication and passes it to genuine `00BD0C30` registration
on the second manager. Normal decrement/leave still uses the first captured
section. Guard cleanup remains armed through `LeaveCriticalSection`; success
then disarms guard and base cleanup and returns the original receiver.

On an ordinary Source C++ escape before guard arming, cleanup invokes genuine
`00412430` on the original receiver. After arming, it invokes genuine
`00411EE0` on the captured guard before the base reset. The established private
render-service RAII pattern filters a second escaping MSVC C++ cleanup
exception to `std::terminate`; other hardware/SEH paths remain outside this
contract. Published values and completed provider effects are retained. The
constructors add no rollback, unregister, publication clear, retry, receiver
allocation/free, callback, facade, private manager or virtual dispatch. The
genuine manager getter retains its existing lazy manager creation policy.

After the base succeeds, the derived Source API writes `00D685F4` at `+0`,
raw DWORD bits `40000000` at `+4`, and zero at `+8`, then returns the original
receiver. These original profile words identify the Native profiles; they are
not callable rebuilt C++ vtables.

The header specifies stable borrowed cell bindings, disjoint receiver/context/
cell storage, valid four-byte-aligned receiver word accesses and storage
lifetime, valid raw manager/vector/section storage through cleanup, and the
absence of conflicting concurrent accesses. These Source signatures add
arguments and are not the original ECX/EAX/plain-RET callable ABI.

## Fresh emitted code and cleanup review

Five complete Win32 objects compiled successfully with the actual configured
`bsp_core` Release command flags, including `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD
/fp:strict /std:c++17`. The generated project, full command log, compiler
identity, per-object commands, dependency lists, complete objects and dumpbin
outputs are retained. Only input/output/include paths were relocated; compiler
version, dependency and class-layout diagnostics were added. No executable
was linked or run, and no new test was added.

The candidate object has 19 sections, 75 exact symbol/AUX entries and 42
relocations. All nine code sections decode completely: 695 bytes and 241
instructions, including both public constructors, private RAII destructors,
depth/filter/unwind helpers and the generated exception funclets/handlers.
The public base body is 189 bytes; derived is 209 bytes. Release optimization
inlines the base schedule into the derived body. Their first 169 physical
bytes match, and derived profile/field stores follow the successful leave
sequence. There is no claim of an emitted derived-to-base CALL relocation.

Both emitted schedules retain the two genuine manager calls, then-current
publication load, registration and first captured section release. Both Source
unwind tables contain state `1 -> 0` guard cleanup and `0 -> -1` base cleanup.
The complete funclet relocations resolve to the private destructors and onward
to genuine `00411EE0` and `00412430`. The guard-unwind body retains its actual
Source exception filter and termination import. The eight-byte guard's profile
and section offsets are `0/4`; the two context bindings are at `0/4`; the
Win32 section occupies `18h` bytes and its extra depth word is at `+18h`.

The physical compiler timing is explicitly qualified. The Source base guard's
armed byte is stored before the base profile, but the compiler's EH state-zero
word is stored after that nonthrowing raw profile store and before the first
getter call. State one and the guard's armed byte remain through leave. Normal
disarm stores and no-op destructor calls are optimized away. These are accepted
Source C++ exception-domain observations, not original FH3/private-spill,
OS-registration, cookie or hardware-fault equivalence.

## Full provider and input provenance

The static graph starts from every candidate code section and follows exact
section/symbol-index relocations through five fresh objects and fifteen whole
members of the retained Source746 `bsp_core.lib`. It covers 175 sections and
444 indexed edges; the 150 boundary records retain actual CRT/Win32 externals
and weak-alias qualifications. Across all twenty complete objects, all 16,530
code bytes decode to 6,090 instructions. Whole provider objects and their
complete sections, relocations, symbols/AUX and decoded code are retained,
including the direct providers' accepted library members alongside fresh ones.

The fifteen library members map to the retained full current provider Source
and baseline Git snapshots. The complete quoted-include closure has sixteen
implementation roots, 48 files and 63 edges. Compiler dependency capture adds
196 actual files. Six actual SDK/CRT libraries and the compiler/dumpbin/vcvars
files retain provenance; those libraries were not newly linked. External CRT
and Win32 service behavior remains a boundary, not a manufactured adapter.

The packet preserves all 750 Source746 input/artifact pins, including the
entire 73,930,550-byte accepted library and its prior executable/map/test log;
those old execution results are context, not fresh tests. It also retains all
605 predecessor pin references, all 420 files from the accepted constructor
readiness closure, and 429 current/Git baseline inputs. Three readiness files
(`game_hosts.cpp`, `sound_configuration.hpp`, `sound_startup.hpp`) have only
CRLF/LF checkout differences; exact old bytes and current bytes are retained
separately. No semantic Source difference is hidden by that qualification.

Complete retained Native constructor evidence is replayed offline: 145-byte/
40-instruction base and 32-byte/9-instruction derived. No fresh Native query,
window, installed-image read or Ghidra mutation occurs. The original ordinary
Source readiness and prior constructor/tail reviews remain part of the inputs.

Preparation's strict-byte failure on an existing line-ending difference and
the first audit's ambiguous substring symbol lookup are retained with their
failed helper versions and logs. The successful preparation requires normalized
equality and preserves both exact byte forms; the audit now selects exact
symbol prefixes. All five actual compilations succeeded on their first run.

The read-only verifier replays full pins, Git/source provenance, original
retained constructor decoding, complete COFF/AUX/relocation indexes, every
Source code byte, indexed graph edges and reachability, Source cleanup-table
evidence, and every immutable ZIP member hash/CRC. The report and out-of-band
verification receipt pin the archive without self-reference. Normal candidate
CMake/build/tests, actual ABI, Native exception execution, allocated-owner
startup wiring and gameplay remain for separate integration or future proof.
