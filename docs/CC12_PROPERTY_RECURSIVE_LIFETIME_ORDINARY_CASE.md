# Recursive property lifetime: one ordinary Source case

One explicitly approved standalone Win32 Source case passed with exit code 0.
The executable ran once, produced both expected witness records, and returned
normally. Post-run file checks passed. This adds **one ordinary Source case
execution**, with no additional Original function, Source function, fragment,
ABI, whole-application, startup or gameplay credit.

The exact image is 235,520 bytes, SHA-256
`bd85a3d93e1f34e8212a46b99e1c85d606015715bd104679e9cec573c717f10a`.
The frozen case Source is SHA-256
`9a193e14ddb3a2864bc42599503d49bfadeca8b1d5fe9232a956896022bfd3ca`.
The run used the separately reviewed one-process executor without modification.
There was no new compile, link, debugger session, retry, Source edit or test-suite
change in this execution packet.

## Admission and executed case

The prior primary review accepted the complete linked image, all six static
gate receipts and the finite execution mechanism. Its receipt is SHA-256
`d11bf6a53d884235199179b2bd0a51d87c5bf299535f2644b9f3347858d13b4a`.
The separate one-process approval is SHA-256
`33135d7edf142a0f5c11e08e0f8ecbf37c266a7c48be41e94f131fefab5d3f22`.
Both authorities and the original approval bytes are retained in the run family.

The unchanged case used the actual canonical String and physical-pool owners,
their real fallback storage, real property-node startup registration, real
empty-bag and type-6 producers, complete lookup/publication providers, and the
accepted recursive lifetime context. Its owning graph was
`P -> R1 -> C -> R2 -> G`, with G empty and two genuine property nodes.
The real query strings and copied node keys occupied four buffers in the same
canonical string pool.

The only top-level recursive-lifetime call was the ordinary Source bag scalar
with flags 2. The case then checked the still-live P, the live node-page free
metadata, the live returned-key ring metadata, and the same canonical owners.
It freed the retained P once, trimmed the empty node page using the complete
accepted Source provider, checked the surviving pool/table/list/critical-section
state, and returned normally. Returned node/key payloads were not read. The
trimmed page and stale table entry were not read after trimming.

The six accepted Source bodies attributed to `008F3F30`, `004E6730`, `008F0DE0`,
`008F0640`, `008F59E0` and `008F5410` were already bound to this exact image as
717 complete bytes, including the release body's real switch data. Their
control flow explains the recursive schedule. This execution does not introduce
instrumented per-function call counts or additional function-level credit.

## Observed result

The exclusive start marker is `2026-10-09T00:36:03.803906+00:00`; the process
receipt is `2026-10-09T00:36:03.854292+00:00`. PID 119076 exited with code 0,
without timeout. Stderr is empty. Stdout contains exactly the ordered phases
`live_graph` and `payloads_released_and_trimmed`.

The saved root addresses below are integer witness values, not pointers used
after the process exited.

| Role | Saved address bits | Allocation bytes |
| --- | ---: | ---: |
| P | 10284616 | 276 |
| C | 10269944 | 276 |
| R1 | 10256632 | 56 |
| R2 | 10256312 | 56 |
| G | 10270232 | 276 |

All five allocation intervals are distinct and nonoverlapping. The reported
return value is P's original `10284616`. The terminal record reports five roots
released, four keys returned, zero node pages, table `10243520` retained at
capacity 32, and the same pool/list owner `8819472`. The critical-section bytes
remain unchanged. The canonical property/query fallback addresses are
`10267756` and `10267757`.

These terminal records follow the case's internal checks of the live state.
They are not a heap-free event trace. The registered callback address was
reported as a Source function identity, not as an observed callback hit. Normal
process exit after genuine registration is observed; callback order, destructor
counts and individual free order are not instrumented. String infrastructure
was intentionally retained until process reclamation, so this is not a claim
of leak-free global shutdown.

## Post-run bookends

The executable, case Source, executor, recipe, all six gate receipts and all
17,403 prior static-review artifacts remained byte-identical. The earlier
14,353-file failed-link seal and the 10,364-file revised-design seal were also
independently rehashed and retained unchanged. Their zero-execution statements
remain historical statements about those earlier snapshots.

All 3,062 current input checks passed, covering 2,203 unique files, including the
657 CPP authority, the 1,411 project Source pins, the consumed headers/SDK inputs,
third-party read inputs, and configured toolchain/environment candidates. All
six current physical Win32 runtime DLLs, their retained images, and the installed
API-set schema remained unchanged. This file identity bookend is not a trace of
the DLLs loaded by the process.

The accepted historical limit remains: 24 consumed headers have pre-link
physical images but no retroactive original compilation preimage. The consumed
Core is the explicitly accepted immutable Registry archive. There is no fresh
all-Core compilation claim, no unselected property-arm coverage, no failure-path
coverage, and no Original register/class/FS/SEH ABI claim. The faithful game
reconstruction goal remains unachieved.

## Evidence and replay

The tracked machine report is
`reports/cc12_property_recursive_lifetime_ordinary_case.json`.
The separate run family is
`local/cc12_property_recursive_lifetime_execution_design_update/execution_attempt01/`.
It contains the unchanged raw process receipt, exact stdout/stderr, approval,
post-run bookends, a separate artifact manifest and final receipt.

Run `ordinary_case_reader.py --verify --check-current` from that family to replay
the sealed run, all three earlier seals, the witness comparisons and current
input/runtime checks. The reader only reads files. After unrelated Source work
changes the current checkout, `--verify` alone replays the retained historical
evidence without pretending those later current files are unchanged. Neither
mode launches the probe. The one execution attempt is consumed; no retry is
authorized by this packet.
