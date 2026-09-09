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

## Concrete dispatch

The durable assignment ledger is [config/parallel_work.json](../config/parallel_work.json).
Address anchors and filenames remain usable when a refreshed inventory changes
segment IDs. Named but unfinished routines are explicitly eligible.

| Work packet | Native anchors | Independent deliverable | Integration boundary |
|---|---|---|---|
| Raw inflater stream | `00bbc060`, `00bbc140`, `00bbbf00`, `00bbc1d0` | Buffered read/seek contract and typed adapter design | Existing source interface and pinned zlib; MPKG parsing stays separate |
| Single-line font layout | `00ab9fd0`, `00aba270`, `00ab6bd0`, `00ad4480` | Scalar width/advance/alignment contract before wrapping/batching | Existing FontData and glyph writer; archive loading is not a prerequisite |
| Native preload policy | `00be7ab0`, `00bde9c0`, `0073d410`, `00686380` | Identify actual startup/runtime cache selections and flags | Existing mounted-stream/FileStore interface; shared probe edits stay with primary |

Workers initially own separate evidence documents, audit reports and ignored
export directories. The ledger reserves separate future source/header files;
that reservation is not a claim that code has been written. Each packet records
its implementation gate and validation boundary. Once reviewed, implementation
can proceed independently against the agreed interfaces, then land in a small
integration batch.

The inflater and font investigations are complete and reviewed; their bounded
implementation contracts are in [INFLATE_STREAM_READ_SEEK.md](INFLATE_STREAM_READ_SEEK.md)
and [FONT_LAYOUT_BOUNDARY.md](FONT_LAYOUT_BOUNDARY.md). The primary agent has
applied their selected descriptive names/evidence to Ghidra, preserving previous
comments and library names. These are analyzed contracts, not new reconstructed
C++ or a measured speedup. Native preload findings are recorded separately in
[VFS_PRELOAD_BOUNDARY.md](VFS_PRELOAD_BOUNDARY.md).

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
