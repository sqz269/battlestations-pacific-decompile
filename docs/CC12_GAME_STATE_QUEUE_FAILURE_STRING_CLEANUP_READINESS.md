# CC12 game-state queue failure string cleanup readiness

`004072D0` is completely reviewed within the requested 256-byte gate:
38 bytes at `004072D0..004072F5`, 14 decoded instructions, and 14 matching
live listing rows. There is no missing listing range. The existing raw SBO
destroy Source already follows the observed inline/heap split, free call and
ordered three-field reset. **No new C++ implementation is needed for this
local behavior; no dormant adapter or production queue binding is approved.**

The recovered provider consumes an ECX receiver and returns with `RET0` and
concrete EAX=0 after normal completion. Its free-provider ABI and ownership
remain external contracts. The selected handler's runtime premise
`EBP = caller entry ESP` also remains external: this function never reads EBP
and cannot establish the required frame rebasing.

The Source baseline is worktree commit
`7a4d3040d5978a80925aa6ddcdee001038371eb2`, after merging current main.
All 4,206 Source files, thirteen accepted overlays and current Source manifests
are byte-identical to the preceding handler packet. Its whole report is a
direct dependency; the capture records the handler's then-pending Root status.
Root subsequently accepted handler worker
`53b4bc9476eb361b298e8bda7eaec985ca829bac` in commit
`7153a978745c25a989d90c1942477f8730cbddcb`, with status
`accepted_selected_cleanup_routing_only`. That whole primary document/report
is separately pinned without rewriting the frozen evidence or relaxing its
runtime frame and ownership limits.

## Complete local behavior

Let `S` be this provider's entry ESP and `Q` its entry ECX receiver.
`PUSH ESI` at `004072D0` saves entry ESI at `S-4`.
`MOV ESI,ECX` at `004072D1` captures Q across the optional free call, subject
to ordinary external nonvolatile-register preservation.

`004072D3` reads the DWORD capacity at `Q+18h` and compares it with 16.
The carry branch at `004072D7` selects the common reset when the capacity is
**unsigned below 16**. This inline route does not read the heap-pointer field
or the length field and does not call free.

For capacity at least 16, `004072D9` loads the DWORD pointer P at `Q+4`,
`004072DC` pushes P, and `004072DD` calls `00BF65AC`, currently named
`_free`. The pointer is captured before the call; no header field is reset
first. There is no null-pointer guard, length check, ownership validation or
capacity-range repair. A null P still reaches the free call if its load
succeeds and the capacity selects the heap route.

On normal free return, `004072E2` discards its one stack argument with
`ADD ESP,4`. The caller does not reload capacity or the heap pointer. Both
paths then execute exactly this common sequence:

| Address | Instruction effect | Write width |
| --- | --- | ---: |
| `004072E5` | `XOR EAX,EAX` | register only |
| `004072E7` | capacity at `Q+18h` becomes 15 | DWORD |
| `004072EE` | length at `Q+14h` becomes 0 | DWORD |
| `004072F1` | first inline byte at `Q+4` becomes 0 | byte |
| `004072F4` | restore entry ESI | stack/register |
| `004072F5` | `RET`, with no argument-pop immediate | stack/control |

The leading DWORD at Q and the remaining union bytes `Q+5..Q+13h` are not
locally written. On the heap route, the final byte store does not clear the
entire old pointer representation. Capacity 15 selects inline storage after
the normal reset; this review does not replace the actual writes with
whole-object zeroing or a full pointer clear.

Q remains captured in ESI across free under the stated provider premise.
Changes made through provider reentry or aliases are not reread before the
common overwrites. The routine has no guard against an invalid heap pointer,
an incompatible allocator domain, or a free operation that invalidates the
receiver itself. Valid raw-object lifetime and matching deallocation
ownership are caller/provider obligations.

## ABI, normal paths and failure boundaries

The exact body has two explicit memory-read operands: capacity at `+18h`
and the heap pointer at `+4`. It has three explicit memory-write operands:
capacity DWORD, length DWORD and the first inline byte. Implicit stack
accesses are separately accounted for. There are two push sites, one pop,
one unsigned branch, one external call and one RET.

The complete local control-flow replay has two normal-return paths. The
inline path visits ten instructions; the heap path visits all fourteen. Both
arrive at RET with ESP=S and return with ESP=S+4. The provider pops zero
ordinary argument bytes. The heap call uses an explicit external capsule:
callee pop 0, followed by this body's observed caller cleanup of four bytes.
Retained `SINGLETON_LIFETIME` documentation identifies `00BF65AC` as a cdecl
CRT free boundary, but its body and complete ABI were not freshly inspected.

ESI is restored after the common stores. EBX, EDI, EBP and EDX are not
locally written. ECX is locally unchanged; the heap call may clobber volatile
ECX/EDX. Ordinary free-provider preservation of captured ESI and other
nonvolatile registers is an explicit premise. No ordinary stack parameter
or incoming EDX value is consumed as a semantic input.

EAX is concretely zero on either normal return because the common XOR follows
the optional free call. This is machine-state evidence, not a newly inferred
semantic return type: the existing C++ Source function is void. The same XOR
sets CF=0, OF=0, ZF=1, SF=0 and PF=1; AF is undefined. Later MOV/POP/RET
instructions do not alter those arithmetic flags. No callable flags interface
or original ABI bridge is introduced.

There is no local FS registration, exception guard, catch, rollback or
alternate failure return. If a capacity/pointer load fails, no header store
has occurred. If free fails to return normally, local argument cleanup,
field reset, ESI restoration and RET are not reached. A fault during the
three ordered stores can leave earlier writes visible. Runtime fault/unwind
behavior and the free provider's failure semantics remain outside this body.
Source `noexcept` does not establish native exception interoperability.

The live listing covers every decoded byte and includes the post-call
`ADD ESP,4` and all reset/return instructions. No missing CALL_RETURN-related
range was identified, so no broader gap/flow investigation or listing repair
was performed. Existing decompiler comments mention older build and fixture
results; those comments are captured metadata, with no fresh validation
credit in this packet.

## Existing Source ownership and handler context

`include/bsp/native_legacy_sbo_string.hpp:9` defines the existing raw storage:
preserved first DWORD, union at `+4`, uint32 length at `+14h`, uint32 capacity
at `+18h`, and intended Win32 size `1Ch`. Its destroy declaration is at
line 41. `src/native_legacy_sbo_string.cpp:56` implements the same local
sequence: test capacity at least 16, free the stored heap pointer, then write
capacity 15, length 0 and first inline byte zero. It adds neither an automatic
raw-storage destructor nor extra byte clearing.

The matching Source allocation path already exists. At
`src/native_legacy_sbo_string.cpp:47`, its allocation wrapper passes object
byte counts to `singleton_lifetime_allocate`. Growth obtains a replacement
through that wrapper; `finish_growth` at line 30 stores the returned pointer
in the same union and publishes capacity/length. At
`src/singleton_lifetime.cpp:51`, the canonical service uses
`std::malloc(host_bytes)` with the documented `_callnewh` retry behavior;
its free service uses `std::free`. These are a matching host CRT pair for
Source-owned pointers. They do not authorize deallocating arbitrary original
game pointers or pointers from another allocator domain.

This fresh body review supports reuse of that existing Source destroy/service
pair for valid, matching Source-owned storage. It does not add a compiler
comparison, Native CRT implementation proof, or binary interoperability
guarantee. The Source function takes a typed reference and returns void; the
recovered original ECX/RET0/EAX0 behavior is retained separately.

The preceding handler packet establishes the selected action
`LEA ECX,[EBP-50h]; JMP 004072D0`. That action's eight bytes and the earlier
failure caller's 105 bytes are retained for identity checks only, with no
fresh decoding here. The whole accepted caller/handler evidence remains the
basis for these relationships:

| Item | Retained relationship |
| --- | --- |
| Failure caller entry ESP | `S_caller` |
| Completed temporary | `S_caller-50h` |
| Exception owner | `S_caller-34h` |
| Selected action receiver | `EBP-50h` |
| Required runtime relation | `EBP = S_caller` modulo 2^32 |

Under that external relation, this provider consumes the completed temporary
selected by state 0. The retained caller publishes state 0 after counted
assignment returns and before logic-error construction; that order remains
unchanged. This provider's observed RET0 closes the local leaf's argument
cleanup amount after the action's tail jump. The runtime still supplies the
return-address/stack context and EBP rebasing.

The exception owner's member string, owner-wide destructor and constructor's
base-only failure guard are different scopes. No exception-owner cleanup or
native throw transfer is inferred from this direct string helper. A portable
scope guard or host throw would not establish the original EH register ABI,
native catch matching or throw-owner destruction.

The full current Source search includes 1,118 canonical string-allocation/free
matches, alongside the prior complete queries. General raw queue
producer/provider/order/owner binding remains open under Root's semantic
coverage correction. This packet changes no automatic adapter, current or
captured profile owner, cancellation, deferred work or shutdown lifetime.
The thirteen accepted overlays and startup/typed-model Source manifests are
unchanged. Historical application-object census qualifications yield no new
build, fixture or unchanged-object comparison credit here.

## Retained proof and reproduction

The report is
`reports/cc12_game_state_queue_failure_string_cleanup_readiness.json`.
Preserve it and the complete relative tree
`local/cc12_queue_failure_string_cleanup_owner_readiness/` for standalone
replay. Portable verification requires no previous packet directory, original
game installation, Ghidra or Git.

Seven typed live CLI queries retain prototype, xrefs, callers, callees,
all 38 body bytes, complete disassembly and complete pseudocode. Each uses
the client verification for configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 and image base `00400000`; autostart is
disabled. Xref and caller queries are each capped at 30 and are retained as
bounded samples, not exhaustive censuses. The selected handler relationship
comes from the whole prior report. The free/CRT/throw runtime and other
provider bodies remain unopened.

The complete original PE and complete baseline/current Source evidence are
retained. The selected Source closure contains 401 files, 26 roots and 734
quoted include edges; the separate typed-model-header closure contains 40
files and 57 edges. Fourteen complete-corpus queries retain 2,273 matches.
The audit reproduces 45 Source excerpts, 13 documentation excerpts, all 14
fresh instructions, every explicit memory operand, both normal stack paths,
the free target displacement and the caller/handler premise. Twenty whole
dependency Git files and twenty support Git files are pinned, plus the two
separate Root handler acceptance files. Prior Root proof trees are not traversed.

Run `python local/cc12_queue_failure_string_cleanup_owner_readiness/verify.py`
for the guarded artifact-only replay. It rejects reads outside this retained
root/report and rejects writes, subprocesses, network use, dynamic library
loading and filesystem mutations after guard installation. It recomputes the
full audit with the helper's final write/print statements removed and requires
exact JSON equality.

The separate `--original-worktree` mode checks the actual Git objects, all
4,206 current working Source files, accepted dependency ancestry/report
identity and the exact two owned outputs. The physical relocation helper
copies the pinned artifacts and exact report to a fresh directory and invokes
the same guarded verifier. These are evidence-integrity checks, not
application tests, fixture execution or Native runtime validation.

Root can retain the canonical Source allocation/free service as the declared
provider contract or commission a separately bounded matching provider review.
This packet opens no further Native range. No C++, compiler, new test, fixture,
runtime, SDK/OS/game action, Ghidra mutation or listing repair occurred.
