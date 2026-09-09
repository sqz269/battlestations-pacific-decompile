# Parallel reconstruction work

Use the partition report to find nearby code, then dispatch a small function
family with a concrete input/output contract. The corrected report does not
establish an independent implementation schedule: in the reviewed baseline,
84 of 87 segments are in
one strongly connected component even before named routines, indirect calls,
shared state and weak edges are considered. A single critical dependency can
matter despite falling below the report's threshold.

This session has four concurrent slots: the primary agent integrates work and
three workers investigate or implement independent families. The first review
used one worker to audit scheduling, then reassigned that slot to VFS preload
policy while the other two continued archive-stream and font-layout work.
No throughput multiplier or completion date has been measured.

After the latest saved function definitions/names, the refreshed graph groups
23,937 candidates into 85 segments, with 82 in its largest strong dependency
cycle. These current segment IDs replace the earlier snapshot's IDs; the
87-segment figures above describe the historical method-review baseline.
Packet function addresses and file ownership remain the scheduling contract.

The continuing-goal instructions in `AGENTS.md` now explicitly request this
one-integrator/three-worker capacity and refilling useful independent packets.

## Concrete dispatch

The durable assignment ledger is [config/parallel_work.json](../config/parallel_work.json).
Address anchors and filenames remain usable when a refreshed inventory changes
segment IDs. Named but unfinished routines are explicitly eligible.

| Work packet | Native anchors | Independent deliverable | Integration boundary |
|---|---|---|---|
| Archive loading | `00bb9920`, `00bb8d60`, `00bbc140`, `00bb9d90` | Provider manager and startup scans integrated; native stream lifetime next | Existing source interface and pinned zlib; shared mount wiring stays with primary |
| Font layout/context | `00b31090`, `00b318b0`, `00b2ebb0` | Owned geometry and parameter table integrated; effect-cache ownership next | Existing font/material contracts; shared renderer/compiler APIs stay with primary |
| Preload/pending I/O | `00509190`, `005092e0`, `00737a50` | Manager/FileStore requests integrated; outer loader/frame lifetime next | Existing manager pump and stable factory; no invented cancellation or failure callbacks |

The inflater and font packets now have bounded C++ implementations in disjoint
source/header files. Each worker also reviewed the other's implementation.
The primary integrated both into the existing build and D3D9 probe: MSVC Win32,
both existing CTests, a 150,123-byte buffered inflate fixture, installed font
layout/geometry, and the existing real font draw pass. See
[inflater implementation](INFLATE_STREAM_IMPLEMENTATION.md),
[font implementation](FONT_SINGLE_LINE_IMPLEMENTATION.md), and the
[integration evidence](../reports/parallel_implementation_validation.json).
No native ABI or original-game equivalence is established by these checks.

The next integration now includes the bounded MPKG parser/materializer, wrapped
font layout and the five-script startup preload policy. Workers implemented
disjoint files and independently reviewed code/fixture evidence; the primary
integrated shared APIs, the build and existing probe. One synthetic archive
checks all three entry routes, and one installed wrapped-text scenario draws
three lines. The preload scenario checks the exact native order/flags and all
140,625 installed script bytes. No new test targets were added. See
[integration evidence](../reports/parallel_entry_validation.json).

The next implementation batch is integrated: mounted MPKG factory/open and
current-VFS reopening, retained font geometry, real physical overlapped reads,
and physical/FileStore/manager enumeration. Workers implemented disjoint files
and cross-reviewed other lanes. Review fixed provider flag-byte truncation
and callback aliasing of the pending pump error output. Existing synthetic
archive and installed font/preload scenarios were extended; one actual-file
pending-read scenario was added to the same probe, with no new test targets.

See [current integration evidence](../reports/parallel_provider_validation.json),
[MPKG provider](MPKG_PROVIDER_IMPLEMENTATION.md),
[font owner](FONT_GEOMETRY_OWNER.md), [pending reads](PHYSICAL_PENDING_READS.md),
and [provider enumeration](PROVIDER_ENUMERATION.md). The required Win32 build,
both CTests and full D3D9 probe pass. Real archives, original object ABI and a
runnable game remain unvalidated.

The current batch integrates the startup provider manager and both package
scans, manager/FileStore pending dispatch and the borrowed material parameter
table. A nested archive checks the second scan and failure continuation. An
installed request checks deduplication, completion and cached bytes. Actual
compiled font metadata binds five pixel constants with unchanged 74/900-pixel
draws. The Win32 build, both existing CTests and full probe pass; see
[current integration evidence](../reports/startup_manager_validation.json).
Workers independently reviewed the other lanes. One automatic destructor tag
was corrected to an allocation helper using its data flow; other library tags
were preserved. Next packets cover archive stream lifetime, effect-cache
ownership and outer loader/frame pumping. Capped tools/bsp.py queries and
sharded ledgers provide their context.

## Rules that keep parallel work useful

- Prefer a few related functions with one reviewable result over a whole segment
  of hundreds of functions. Grow a packet only when its actual dependencies call
  for it. Partition endpoints are function starts, not complete byte boundaries.
- Workers can read shared types and constants. They own disjoint output files
  and analysis ranges; coordinate extensions and hand off shared-type changes.
  The primary agent owns common headers, CMake, probes, ledgers, Ghidra writes,
  builds and integration. Preserve existing library names and comments.
- Return addresses, original ABI, verified byte spans, external contracts,
  uncertainties and a precise next implementation. Labels and plausible C++
  types alone are insufficient evidence. Resolve divergent findings before
  integrating either interpretation.
- Use existing checks and probes first. Add only the smallest case needed for
  a concrete behavioral risk; no test framework or suite was added for this
  scheduling change. Report exported, analyzed, reconstructed, build-tested,
  fixture-tested, ABI-compatible and game-validated states separately.
- Refill a worker slot with the next bounded ready packet while other work
  continues. Optimize for reviewed subsystem progress; do not maximize rename
  counts or produce unrelated fragments just to keep workers occupied.

## Reproduce the corrected report

With the current saved snapshot under `exports/bsp/`:

```powershell
python tools/callgraph_sweep.py
python tools/partition_candidates.py
```

The optional analysis tools use Python, pefile, Capstone and NetworkX. The sweep
records executable/function-list/tool identity and Capstone version. Partition
output records input hashes, snapshot identity, generator/executable hashes and
NetworkX version. A changed snapshot requires both steps again. The linear
sweep remains a heuristic; it does not recover indirect dispatch or validate
function reachability and pointer-table identities.

[Method review](PARTITION_METHOD_REVIEW.md) explains the proposal's issues and
the applied fixes. [Validation data](../reports/library_inventory/partition_validation.json)
records the same-input baseline comparison: 458 qualifying edges restored, 38 waves and
eight purity fields corrected, with all 87 function groups unchanged. The
reported 1,554 provisional bookmarks and 54 named-but-incomplete addresses are
scope reminders, not automatic exclusions or completion credit.
