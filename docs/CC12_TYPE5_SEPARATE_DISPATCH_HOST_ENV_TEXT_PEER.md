# Type5 separate dispatch-host environment TEXT peer review

The V5 candidate changes exactly two expressions so the nested PowerShell host
uses the external-host environment while its requested Python or compiler child
retains the effective child environment. Independent passive review passed.
This is **Source=0**, without Root adoption or V5 dispatch execution evidence.

Packet: `cc12_type5_separate_dispatch_host_ENV_TEXT_peer`. Zero addresses; only
this document and `reports/cc12_type5_separate_dispatch_host_ENV_TEXT_peer.json`
are owned tracked outputs. The worker fast-forwarded to Main
`3b9c2575e65bd32a0f45f9727ee7a4a06107054c` before claiming the lease.

## Exact inputs and independent freeze

The original immutable `Main/local/type5guardReview01` was independently
reverified at freeze, before and after review, and before sealing: all 11,689
artifact sizes/hashes and exact 11,690-file membership passed. Its seal is
2,056,797 bytes with SHA-256
`31ca9132e17e1c1c3fda24bd4dca34bc333346ee84737f6c9454027ebca01241`.

The fresh ignored family is `local/t5hostEnvPeer` in the worker worktree. It
contains fresh copies of the 12 sealed V4 payloads, 12 V5 payloads, the original
Root seal, and exactly seven additional fixed data files from
`Main/local/type5select01`: `guard_v5_derivation.json`,
`guard_V5_PowerShell_AST.json`, `profile_probe_v3.ps1`,
`private_profile_probe_v3.json`, `private_external_environment.json`,
`private_effective_child_environment_v2.json`, and
`private_root_python_environment_v2.json`. The changing Root workspace was
neither snapshotted nor treated as sealed. The 32 original/frozen pairs remained
unchanged through the final bookend. Earlier sealed worker families were not
modified or replayed.

Before optional analysis imports, the worker freshly froze 2,102 dependency
files across 3,588 membership records and 16 scopes, including literal Python
stdlib/DLL membership, native companions, package metadata, and startup inputs.
The own Python utility ran with `-I -B -E -S`, optimization zero, and an empty
private bytecode prefix. All 46 loaded file origins matched this fresh closure.
Every pre/post/final bookend checked 2,134 original/frozen pairs, all original
Root artifacts, the 16 analysis scopes, and the unchanged private process
environment. No environment values are published here.

## Two changes and preserved behavior

1. `complete_guard.dispatch` passes
   `environment(s['private_external_environment_pin'])` as the `env` argument
   when launching `pwsh`, replacing `child_environment` at that one call.
2. `external_dispatch.Assert-Selection` always obtains its host environment
   from `private_external_environment_pin`, replacing the role-dependent choice
   between the external and effective child profiles.

Each change is exactly one byte-text replacement. The other ten payloads are
byte-identical. The only Python AST change is the first expression above; all
other functions and module-level statements match. All 170 Python assertions
and 383 explicitly named guard calls retain their exact AST and order.
PowerShell parsing returned zero syntax errors; all 57 throw statements, all
25 `Assert-*` command texts, and mandatory parameters remain unchanged. Its
only changed function is `Assert-Selection`.

The recipe still derives `PATH` with the selected MSVC/SDK prefix and selects
the exact derived `INCLUDE` and `LIB`. It still passes that environment into
`dispatch`; dispatch still checks it against `effective_child_environment()`
and writes the effective profile pin into the nested request. The driver still
requires a root-entry request to select the root Python profile and a nested
request to select the effective child profile. It clears the child process
environment and copies that request-selected profile into it. The actual
Python environment comparison is unchanged.

Root adoption, execution-hold, full content/membership, payload, receipt,
host/driver, argv, request identity, request-keyed snapshot naming, and child
admission checks remain intact. External preflight precedes child process
start; the unconditional postflight still checks the external host profile
and its original full ambient environment. Timeout, completion, log identity,
and postflight bodies are unchanged. Their preservation is source evidence,
not a new process-cleanup or runtime guarantee.

## Root's recorded inspector evidence

The worker parsed Root's V3 inspector source and independently compared its
saved requested/actual dictionaries as data. Requested dictionaries matched
the three frozen private profile files exactly. All four recorded inspectors
closed with exit code zero. `external_host` matched all 14 keys;
`root_python` and `effective_python` each matched all 16 keys. `nested_host`,
when given the effective profile, differed only in `PATH`: its recorded actual
value prepended the PowerShell executable's directory to the requested value.

These are verified properties of Root's saved V3 observations. The inspector
was not replayed, and the V5 driver was not executed. V5's source-level routing
uses the profile for which Root recorded an exact external-host match while
preserving the separate requested child environment.

## Closure and limits

Own utility attempts passed without failures; their sources and outputs are
preserved. The sealed worker family has 2,157 artifacts plus its seal, exact
2,158-file membership. The seal is 399,523 bytes with SHA-256
`11d194c2b1f047fde3db422cf4b2fe86f240ea1e137d0ca7242639723638afbd`.
After sealing, all 2,157 artifacts and exact membership were independently
reverified. Detailed payload and evidence pins accompany the report.

Root must independently finish its fresh full dependency selection and any
required adoption/receipts. This packet neither fills Root fields nor changes
the old Main content/membership holds. It does not materialize or execute the
candidate, replay prior runtime evidence, invoke a compiler, query
Native/Ghidra or a mapped module, or run a provider, original PE, or target.
There are no C++ or test changes and no ABI/gameplay claim.
