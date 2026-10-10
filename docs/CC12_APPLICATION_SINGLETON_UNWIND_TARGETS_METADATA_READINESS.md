# Application unwind targets: metadata and Source readiness

**NO_CPP_GO; NO_FRAME_ACTIVATION_GO.** The two previously decoded jump targets
have independently complete saved ranges: **`00412430..00412436` (7 bytes)**
and **`00411EE0..00411EF8` (25 bytes)**. This packet admits no Native code/data
bytes, instruction text or decompilation. It establishes finite future body
gates and identifies existing Source providers, without asserting cleanup
semantics from function names or metadata.

Source is frozen at `1c44e1c4e70818d5aa02a53d2427b46eee768dae`: all 4,210 tracked
src/include files, Git blobs and working bytes, plus 34 supporting files. All
Source paths match the preceding action packet. Its 525 pins are physically
copied unchanged, including nested 433/355/287/230/184/124 proofs and qualified
compiler-definition artifacts.

## Typed ownership and listing gate

Only addresses `00412430` and `00411EE0` were leased and queried. Full raw typed
responses, base64 copies, response hashes, prototype summaries and the count
check are retained. Before/after typed records agree exactly at **epoch 95**.
Every Native CLI batch verifies project `bsp`, `C:\Users\sqz269\bsp.gpr`, program
`/battlestationspacific.exe`, x86/32-bit little-endian, 64,730 functions.

| Metadata | `00412430` | `00411EE0` |
| --- | --- | --- |
| Stored name | `BSP_SingletonBase_ResetProfile` | `BSP_SystemSingletonGuard_Destroy` |
| Complete range | `00412430..00412436` | `00411EE0..00411EF8` |
| Stored body bytes | 7 | 25 |
| Range count / returned count | 1 / 1 | 1 / 1 |
| Entry instruction length | 6 | 3 |
| Entry fallthrough | `00412436` | `00411EE3` |
| Entry flow override | NONE | NONE |
| NoReturn / thunk | false / false | false / false |
| Stored prototype | `void __thiscall(void * this)` | `undefined(void)` |

At each address, function-at and function-containing agree with the exact
entry. Instruction-at, instruction-containing and both code-unit views agree
on that entry and length. Both entry default/effective flows are ordinary
fallthrough. No missing owner or discontinuous stored range is reported, and
the two complete ranges are disjoint.

This is not a global exclusion of third-party shared tails or ownership
anomalies elsewhere. No terminal instruction was inspected, so its boundary,
opcode, return behavior and possible transfer outside the range remain open.
The summaries report two/eight instructions and zero calls; a zero call count
does not exclude a machine JMP or prove absence of external cleanup. In
particular, the stale indexed `BF6B43` edge discussed in the preceding Root
acceptance is not used as instruction evidence or a reason to inspect it.

No additional instruction-boundary query was needed to propose the complete
saved hulls. The next packet must refresh their ownership and entry/end
listing boundaries before admitting those exact bytes. No listing, flow,
prototype, no-return, name, comment, ledger or body mutation occurred here.

## Existing Source providers

The frozen Source corpus has address-specific providers with actual raw
pointer parameters. This is Source implementation evidence, separate from
the current metadata-only Native observations.

| Provider | Frozen Source behavior | Build-list evidence |
| --- | --- | --- |
| `destroy_native_generic_singleton_base_00412430(void*) noexcept` | A volatile `uint32_t` store of `CE3818` at the supplied receiver | `src/native_render_service_base.cpp:51`, listed by `cmake/startup.cmake` |
| `destroy_native_singleton_guard_00411ee0(void*)` | Capture guard+4, write `CE37FC` at guard+0; if captured pointer is nonnull, decrement its counter at +18h then call `LeaveCriticalSection`; preserve guard+4 | `src/native_diagnostic_sink_lifetime.cpp:33`, listed by `CMakeLists.txt` |

The guard's Source parameter denotes an actual eight-byte record containing
a native section pointer. Its tracked counter is beyond the native
`CRITICAL_SECTION` storage at section+18h; a typed C++ section projection is
not a substitute. Existing declarations and Source assertions preserve the
MSVC Win32 domain. The reset provider's explicit `void*` argument and guard
provider's explicit record argument are C++ interfaces; saved Native
prototypes do not establish drop-in ABI compatibility.

Eleven whole-Source lexical queries and 17 excerpts are retained. The target
queries return 30 reset-address/provider lines and 61 guard-address/provider
lines across the closed corpus. These counts demonstrate Source references,
not Native callers or behavioral coverage. Build-list membership establishes
that these sources are configured inputs; no build or runtime test ran here.

The retained action bodies at **epoch 92** supply exact destinations and
distinct operand forms: loading `[EBP-18h]` versus forming `EBP-14h`. The
constructor comparison remains **epoch 79** and conditional on the required
incoming frame. Existing Source helper bodies do not establish that frame,
actual EH invocation, exceptional order, target return behavior, or a callable
Application owner/deletion profile. In particular, names and a stored
`__thiscall` annotation are not substitutes for the next exact-body analysis.

## Custody, proof and next proposal

Preserve actual receiver and guard identities, stable raw storage, the actual
+14 member reference, authority cells, borrowed services and escaped
startup/shutdown/failure records. Current publication, selected registration
argument and captured section remain distinct. No new reset, unregister,
free, retry or destructor policy is admitted. `E1AE78` remains a separate
ownership gate; normal callable retirement remains unresolved.

Finite replay validates both complete Source trees, Git blob identities,
supporting files, lexical queries/excerpts, inherited proofs, raw typed
responses, capture order and Source provider facts. Physical relocation copies
only declared files plus the report; its guard denies undeclared reads,
writes, subprocesses and network. Actual Git replay additionally validates
the current/prior frozen blobs and both committed outputs.

Metadata epoch 95, action epoch 92, map epoch 90, descriptor epoch 88, handler
epoch 85 and constructor epoch 79 remain separate observations. No atomic
runtime contract, new build/test result, native execution, ABI equivalence,
Application activation or game validation is claimed.

Next single bounded proposal:
`cc12_application_singleton_unwind_targets_body_contract_readiness`.
Independently claim **`00412430..00412436`** and **`00411EE0..00411EF8`**.
Refresh complete typed ownership and entry/end listing boundaries first;
admit only unchanged complete contiguous hulls, at most **7 + 25 = 32 bytes**.
Missing, changed, larger, overlapping or ambiguous ownership requires a
handoff. Decode only these bodies and compare their operand/control/return
requirements to the frozen Source providers and retained action operands.
Do not follow another callee or external target, read a descriptor suffix,
inspect `BF6B43` or another funclet, read installed CRT/OS/game state, or repair
listing/flow/prototype/no-return metadata. Actual incoming EBP/FH3 binding
remains a separate gate.

Evidence: `reports/cc12_application_singleton_unwind_targets_metadata_readiness.json`
and its declared local finite bundle.
