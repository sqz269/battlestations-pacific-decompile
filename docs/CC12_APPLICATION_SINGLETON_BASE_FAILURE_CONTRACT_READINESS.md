# Application singleton base: publication and failure contract

**NO_CPP_GO for the raw Application owner; NO_FRAME_ACTIVATION_GO.** The normal
singleton-base sequence and its interruption sites are now exact. Exceptional
cleanup remains unproved: the saved handler anchor `00CC7240` has a defined
instruction but no function at or containing it, so it has no complete owned
hull eligible for the authorized body read. No handler body was read and no
function, listing, prototype, flow or no-return property was repaired.

The frozen Source epoch is `8cf81aaf571bcd8f452990161826cfefbb9cfba1` on
`agent/cc12_application_base_failure`. All 4,208 tracked src/include files,
their actual Git blobs, working bytes, and 14 supporting files are retained.
Source is byte-identical after line-ending normalization to the preceding
constructor epoch `c958f65da2436812f995512ed8a4f720e3f79c81`. Root's independent
later record/grid work receives no credit here. The preceding constructor
proof's 184 declared pins, including the nested authority proof's 124 pins,
are copied unchanged and replayed. Their seven selected historical Source/build
epochs and older complete Source corpora remain separately qualified.

## Native admission and ABI

Read-only prototype and typed metadata queries preceded the fresh body read.
The lease was extended to the exact admitted range before listing/bytes capture.
Before/after typed captures agree at modification **79**, project `bsp`, marker
`C:\Users\sqz269\bsp.gpr`, program `/battlestationspacific.exe`, function count
64,730. Every CLI query verifies project/program through the retained tool.

| Entry | Typed admission | Fresh evidence |
| --- | --- | --- |
| `00BEA810` | One complete range `00BEA810..00BEA8A0`, 145 bytes, below 192-byte cap | Full listing and exact bytes; 40 decoded instructions |
| `00CC7240` | No function at/containing anchor; instruction begins there with length 5 | Metadata/prototype only; zero handler body bytes/instructions |

The 145 bytes have SHA-256
`7cf5ea564272d231a6fa4ad3d8f88f1439d3370d31a5310132241cc9fc4eeee2` and exactly
match the saved earlier base record. That equality does not merge observation
epochs. The saved record's installed-PE comparison remains historical; this
packet did not reopen or execute the installed image. The saved 42-byte outer,
22-byte loop-base, and 57-byte caller records retain their earlier provenance.

Assembly takes the receiver in ECX, captures it in EDI, preserves EDI/ESI,
returns that receiver in EAX, and ends with plain RET and no explicit stack
arguments. The existing `undefined BSP_ApplicationSingleton_Construct(void)`
prototype does not encode those facts. The frame links FS:[0], carries the
`CC7240` handler address and a state spill; these are observed setup operations,
not a recovered FH3 action table or a source-language exception guarantee.

## Exact normal sequence

| Site | Observed operation and authority |
| --- | --- |
| `BEA830` / `BEA838` | Store DWORD state 0, then `D68BC4` at original receiver+00. |
| `BEA83E` / `BEA843` | Call `415350`; capture returned manager's actual DWORD+10 section into ESI. |
| `BEA848..BEA85D` | Construct stack guard profile `CE37FC` and saved section; if nonnull, call indirect entry at `CE2218`, then increment that section's DWORD+18. |
| `BEA861` / `BEA866` | Store byte state 1, then unconditionally publish original receiver at `E1AE90`. |
| `BEA86C..BEA87A` | Call `415350` again; reload **current** `E1AE90`, push it, use second manager result as ECX, call `BD0C30`. |
| `BEA87F..BEA888` | If originally captured ESI is nonnull, decrement its DWORD+18 and call indirect leave at `CE2210`. |
| `BEA88E..BEA8A0` | Restore exception chain/registers and return original receiver in EAX. |

The original receiver, first manager, captured section, second manager, current
publication and registration argument are separate identities. The body neither
rechecks an existing Application before its store nor caches the original
receiver as the registration argument. An intervening call can change the
publication; registration can receive another pointer or null. Normal return
still returns the original receiver and does not reload `E1AE90` afterward.
The original section survives manager/publication changes as the release target.
The Source providers identify the two imported calls as Enter/LeaveCriticalSection;
no fresh import-table or OS-provider body was inspected.

## Failure boundaries and retained authority

The following records describe reached direct operations, **not post-unwind
states**. Unknown callees/callbacks may add effects; a faulting operation may have
an indeterminate disposition. Nonreturn does not imply a C++ throw or prove which
original handler actions execute.

| Interruption boundary | Established prefix and unresolved obligation |
| --- | --- |
| First manager resolution or raw+10 read | Initial `D68BC4` store has run; this body has not yet stored `E1AE90`. Manager getter effects and any exceptional base cleanup remain separate. A null manager has no local fallback before dereference. |
| Section entry or tracked increment | Captured section/guard exist; state 1 and publication stores have not run. Entry/depth disposition must not be guessed from a failed return. |
| After publication, during second lookup | Original receiver has escaped through `E1AE90`; outer constructor fields have not run. Later current publication and manager side effects must be observed separately. |
| During registration | Second manager and the then-current publication argument were selected. This caller proves no atomic insertion, rollback, unregister or allocation cleanup. Original receiver can remain exposed even if another/null argument was selected. |
| After registration, during release | Registration returned, but release may be incomplete. Depth decrement precedes Leave; state 1 is still stored. No assumed second release or automatic compensating decrement is valid. |
| Base returns normally | EAX is original receiver. Only now may the previously admitted loop/outer shells write `D68BC8`, byte+04, final table, vector, Game+14 and flags. |

There is no direct publication-clear, saved-old-publication restore, unregister,
free or retry operation in this body. That is not an absence proof for its
unread handler, tables, cleanup bodies or callees. State 0/1 spill writes alone
cannot identify destructors or authorize copying a catch block from a different
singleton. The handler gap therefore blocks a complete failure-retirement
contract and a raw owner implementation.

Any future composition must retain a stable original raw receiver and actual
`E1AE90` cell, plus the actual +14 reference, selected manager/registration
argument and captured guard domain needed to resolve escaped references and
failure records. Neither a null current publication nor a throwing registration
proves that the original receiver is unregistered or releasable. The owner and
services must outlive registration, startup, shutdown, all retained frames and
final observations. No reset, unregister, free, retry or standard C++ destructor
policy is admitted here.

## Existing Source providers and remaining composition

The complete-corpus searches and positive definitions establish these boundaries:

* `native_singleton_publication.cpp` supplies the actual-cell manager getter.
  It captures an initial nonnull publication; otherwise it allocates raw14h,
  constructs, publishes and returns its result. Its own constructor catch frees
  its saved manager allocation. That policy belongs to the manager allocation,
  not the Application receiver. No publication lock or final reload is added.
* `native_diagnostic_sink_lifetime.cpp` supplies `411EE0` over an actual eight-byte
  guard. It captures guard+04 before rewriting the profile, decrements raw+18,
  leaves that section, and preserves +04. A usable provider does not establish
  that the unread `CC7240` cleanup invokes it, or in which states.
* `native_singleton_vector_registration_wrappers.cpp` supplies `BD0C30` with
  actual manager storage and stack argument. Bounds validation precedes the
  current argument's null test; a nonnull argument is passed by stack-slot
  address to append. There is no lock, duplicate check, retain or local cleanup
  frame in this wrapper. Append/growth and returning validation preserve their
  own mutation/failure contracts; registration has no success-status result.
* `GameSingletonHost` borrows `GameNativeStringProcess::manager_01090aa0()`.
  Future composition must share that stable authority. The neighboring resource
  registry getter demonstrates captured-section/current-publication scheduling
  and its own established catch policy; it does not prove Application unwind.
* All four `E1AE90`/base/storage-name search hits are comments. No Application
  authority is created by those hits. `GameStartupHost::application_construct`
  changes projected flags and `constructed_`; destruction clears that boolean.
  No `D68BC4`, `D68BC8` or `CFEAB0` profile appears in the complete Source corpus.
  Existing raw deletion rejects unsupported profiles. Numeric Native table
  identities are not callable Source vtables or a qualified retirement binding.

`GameNativeGameApplicationFrame` retains its caller's actual +14 by reference,
records failures, and enforces its distinct Source retention policy. It does
not create the containing Application owner or cure this constructor's handler
gap. Existing pointer-vector storage at+08 also remains a subobject provider.
The process duplicate `E1AE78`, pooled temporary and Game name copy remain
separate lifetimes; none acquires a producer or retirement policy in this packet.

## Verification and next bounded gate

The finite portable replay recomputes six complete Source searches, 14 bounded
excerpts, current Source/Git closure, the unchanged preceding proof and its
nested authority proof, both typed captures, all 145 decoded bytes and their
saved-byte equality. Physical relocation copies only the declared files and
report; the verifier restricts reads to that exact set and disallows writes,
subprocesses and network access while replaying. A separate actual-Git replay
checks the current and preceding constructor Source corpora/support blobs and,
after commit, the exact two committed document/report blobs and clean worktree.
These are static artifact checks, not tests or executable behavior evidence.

Next proposed packet: `cc12_application_singleton_handler_listing_contract_readiness`.
At `CC7240`, recheck typed metadata first. If it still has no function, propose
read-only exact instruction-context walking from that proven instruction start:
at most eight starts and 64 total instruction bytes, following verified inline
successors only, stopping at terminal flow or a listing gap. This does not infer
a function boundary from adjacency or authorize creating one. A newly present
complete function would still require the 64-byte hull gate. Any derived EH
table, cleanup body, destructor or other callee needs its own finite proposal
before a fresh read. Normal callable Application retirement remains another
open contract. No expansion, repair, C++, build, test, probe, Native/OS provider
execution, frame activation or game validation is authorized by this report.

Evidence: `reports/cc12_application_singleton_base_failure_contract_readiness.json`
and its declared local finite bundle.
