# Native renderer BeginFrame and raw diagnostic record erasure

Addresses: `00B2B200`, `00B15090`, `00B14480`, `00B13630`, `00B13280`,
`00B13180`, `00B10740`. The descriptive record names are hypotheses.

This packet implements all seven bodies against actual Win32 storage. It borrows
the full Reset provider and the same `00F8D39C` publication used by
`native_render_diagnostic_labels`. `NativeStringRawPoolContext` borrows the actual
`01090AA8` pool publication, `01090AA4` small-return gate and canonical raw
`01090AA0` manager publication. There is no separate pool, service, semantic
release callback, general STL implementation or new CRT.

| Routine | Inclusive body | Bytes | Original ABI | Coverage |
|---|---|---:|---|---|
| BeginFrame | B2B200..B2B263 | 100 | ECX renderer; RET; AL=1 only | complete |
| Clear service records | B15090..B150CC | 61 | ECX actual service; RET; no semantic result | complete |
| Erase checked range | B14480..B144FD | 126 | ECX vector; five DWORD slots; RET14; EAX out | complete |
| Copy tail and recompute end | B13630..B13687 | 88 | cdecl six DWORD slots; RET; EAX new end | complete |
| Forward record copy | B13280..B132B7 | 56 | cdecl six DWORD slots; RET; EAX output end | complete |
| Assign raw record | B13180..B1327D | 254 | ECX destination; stack source; RET4; EAX destination | complete |
| Destroy raw record | B10740..B107C2 | 131 | ECX record; RET; no semantic result | complete |

Total: **816 native body bytes**. The new C++ context interfaces do not reproduce
the original register, stack, FH3 or SEH ABI. `B145A0`, the other clear wrapper
calling `B14480`, is consumed evidence and is not reconstructed by this packet.

## Producer and ordered record behavior

`B14A10` constructs the actual `6ACh` service, calls `B0F020` at `B14A31`, and
installs `D5E480` at `B14A36`. The base publishes that object into `F8D39C` at
`B0F076`. The constructor zeros begin/end/capacity at `B14C35/B14C3B/B14C41`,
respectively service `+690/+694/+698`. The vector base is `+68C`; its prefix
word is unused by this path and remains untouched. The diagnostic label at
service `+684` is the existing actual eight-byte native string.

`B13180` establishes a `12Ch` record stride: an opaque `11Ch` prefix followed by
eight-byte strings at `+11C` and `+124`, with length first and pointer second.
The prefix copy is one DWORD followed by five groups of fourteen DWORDs at
`B131A0/B131B5/B131D0/B131E8/B13200`. Each source DWORD is loaded immediately
before its destination store. Source/destination overlap can therefore cascade;
`memcpy`, `memmove`, or an eagerly captured payload would change that behavior.
The source uses scalar MSVC assembly accessors to retain the operation order.
The three-byte Ghidra listing gap `B1319D..B1319F` is the unreachable alignment
instruction `8D 49 00`; it is included in the body span without an analysis edit.

Each string compares exact header addresses. If different, resize with initial
source length and preserve=1, then reload source length. A nonzero current
source length causes a copy using the current destination length, current source
data and current destination data, in that read order. The native `BF7680`
checks overlap at `BF7694..BF769A` and selects a backward path at
`BF7844..BF79E4`; source nonzero copies use `memmove`. Zero-byte library calls
are omitted, as in the existing actual string provider, after retaining the
native header reads. The raw overloads of `41DD40` and `41DD20` allow current
pool-getter exceptions on both allocation and release; see
`NATIVE_STRING_RAW_POOL_CONTEXT.md`. The older `NativeStringStorage::release`
noexcept interface is not used here.

`B10740` captures second data at `+128`, arms state0 before testing that pointer,
and reads size `+124 + 1` only for nonnull data. It calls current `419CC0` before
`BD1510`, even for large blocks or disabled small returns. After that release,
it loads current first data at `+120` and disarms before reading first length
`+11C + 1` or looking up the pool. Neither normal release clears a header.
Every size/address addition uses DWORD wrapping.

The original EH action `CBBC80..CBBC8D` reads the saved record at `[EBP-10]`,
adds `11C`, and tails `41DD20`. Handler bytes `CBBC8E..CBBC97` are
`B8 10 3F DF 00 E9 AB AE F3 FF`: load `DF3F10` and jump `BF6B43`.
`DF3F08..DF3F0F` contains the sole unwind map entry `(-1, CBBC80)`;
`DF3F10..DF3F33` contains FuncInfo magic `19930522`, maxState1, that map,
zero try/IP maps and final EH flags1. Thus a second-string getter failure cleans
the **current** first header; a first-string getter failure has no cleanup here.
The source arms an RAII guard for this state and terminates if a C++ exception
escapes its unwind cleanup. Native handler definition/annotation is centralized
through the integrator; the handler was not a Ghidra function at worker capture.
Before this commit the integrator defined and saved the exact ten-byte handler
as `FUN_00cbbc8e`; a live worker prototype query confirmed `CBBC8E..CBBC97`.
Integration provenance is in `reports/native_renderer_ay_analysis_sync.json`.

## Iterator and frame ordering

`B13280` copies forward until first==last and returns the incremented output;
equal pointers do no work. `B13630` ignores that returned EAX and recomputes
`output + 300 * (signed32(wrapped(last-first)) / 300)`, truncating signed division
toward zero and wrapping the result. Its six cdecl argument slots are first,
last, output and three ignored slots. The caller cleans `18h` at `B144C9`, and
`B1367C` likewise cleans its call to `B13280`. The native tag locals initialize
only their low byte; their padding is not promoted to semantic initialized
state in the C++ API.

`B14480` has five caller slots: out, first.owner, first.record, last.owner,
last.record, confirmed by `RET14` at `B144FB`. It validates only nonnull
first.owner equal to last.owner, permitting a returning CRT handler and without
requiring either to equal ECX. It then compares the record positions. For a
nonempty range it captures vector end, copies `[last,captured_end)` onto first,
reloads current vector end after copying, and destroys `[new_end,current_end)`
in ascending record order. End publication occurs only after all destructors
return. Finally it writes output.owner then output.record and returns the output
address. Output can alias vector storage; the source preserves those store
orders. Native argument/EH-spill aliasing is outside the new source ABI.

`B15090` captures end first, validates current begin against that captured end,
reloads begin, compares it with a fresh end, then passes the captured end and
reloaded begin to the general eraser. Returning validation can make this clear
perform a nonempty tail copy. No end/begin retry or new bounds policy is added.

`B2B200` checks renderer active DWORD `+1998`. Already active skips everything.
Otherwise it captures inhibit `+1D90`, publishes active1, decrements a nonzero
captured inhibit, and calls complete `B2ABD0`. Only after Reset does it reload
inhibit and lost byte `+1D8A`; both zero permit a call through the **current**
device at `+1A10`, vtable `+A4` (`BeginScene`, stdcall device, HRESULT ignored).
It then reloads actual `F8D39C` and clears its records when nonnull. All normal
exits specify AL1 only. A Reset, COM, validation or pool exception preserves
preceding active/inhibit writes and any completed copy/destruction work.

## Native call evidence

The machine-readable report carries each numeric call site, callee, containing
function and original cleanup. Direct calls and the EH tail were checked by
`tools/verify_report_calls.py`; the current-device `+A4` call is explicitly
indirect. Complete owned listings, complete Reset, raw getter/return/resize/
destruction providers, invalid-parameter service, BF7680, both actual service
producers and the second B14480 caller were inspected. Live queries verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` through the standard CLI.

## Verification and limits

Strict owned source compilation passed MSVC Win32 `/W4 /WX /O2 /EHsc /fp:strict`.
The repository's `scripts/build.ps1` Release Win32 build and both existing CTests
(`reconstructed_math`, `native_math_differential`) passed; no repository tests
were added. The log is `local/ay-build.log`. CMake/ledgers/Ghidra edits remain with
the primary integrator. A worker build alone does not enroll the new source in
`bsp_core`; strict compilation and the external fixture compile it explicitly.

The separate `C:/Users/sqz269/bsp-ay-begin-frame` fixture preserves the original
AX fixture unchanged. It reserves a private PE image section before process
startup, embeds a manifest, pins the complete seven native bodies plus 68 EH
bytes, and compares all nine spans to live saved Ghidra bytes and the installed
original PE. Seven exact four-byte operand relocations bind the borrowed global,
handler trampoline, two resize calls, two getter calls and cleanup tail.
Provider entry bridges call the concrete raw pool/manager/resize/return bodies,
current CRT memmove/validation/FH3 and the existing actual Reset providers.

Twenty original/source cases pass, comparing **167,745 normalized state bytes
and 1,829 event DWORDs**. They cover active bypass; inhibit decrement; lost gate;
null/nonempty/fresh service; real hidden HAL BeginScene and ignored failure
HRESULT; BeginScene exception; real HAL Reset and Reset exception; middle erase;
returning owner validation; clear's captured end with a newly extended tail;
forward scalar overlap; exact self assignment; empty copy; real lazy manager
registration failure on second versus first string; post-resize source mutation
through a returning real CRT handler; and disabled small plus large returns.
Defined arena bytes, current small-string contents, complete ring indices,
live/peak counters and normalized queued block order are compared.

The second-getter failure publishes a new concrete pool, then invokes the raw
manager's installed CRT handler. That handler changes the first header and
throws; original FH3 and source cleanup return exactly the changed block into
the new pool's ring. The first-getter failure has zero ring returns. This is
observed provider/EH evidence rather than an injected release or getter stub.
The fixture observes real private Win32/D3D calls and controls documented CRT
validation/COM failure branches. It preserves foreground and focus and closes
its hidden scenes/windows. It does not prove every invalid-address path, huge
signed-wrap range, double-exception behavior, SEH, concurrency, original binary
compatibility or gameplay. The replay report pins source/library hashes and
the exact relocations; the source interfaces are reconstructed and fixture
tested, with final combined-library replay reserved for integration.

## AY integration analysis refresh

The integrator saved all sixteen AY original signatures and reviewed names,
verified full stored bodies and refreshed exports. CBBC8E, CBD436 and C64F13
are ten-byte analysis-only EH handlers defined under leases and the write lock.
Earlier missing-function observations are retained as worker capture history.
Two existing raw string bodies were extended separately; neither those
extensions nor the EH definitions add to the sixteen normal-body count.
Exact combined validation follows separately from worker fixture evidence.

## AY exact merged validation

The exact combined source commit `71ff3f1220fecd9e4e9ff7ad7ffc54b3a38477f4` passed the strict Win32
build, both existing tests and four current-library-only original-byte fixtures.
See `reports/native_renderer_startup_ay_validation.json` for hashes, preserved
captures, case coverage and limits. Earlier pending statements describe worker
capture stages. Whole-game rendering, native ABI and general concurrency remain open.
