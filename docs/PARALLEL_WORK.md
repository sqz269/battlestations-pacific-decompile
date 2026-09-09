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

The continuing-goal instructions in `AGENTS.md` now explicitly request this
one-integrator/three-worker capacity and refilling useful independent packets.

## Concrete dispatch

The durable assignment ledger is [config/parallel_work.json](../config/parallel_work.json).
Address anchors and filenames remain usable when a refreshed inventory changes
segment IDs. Named but unfinished routines are explicitly eligible.

| Work packet | Native anchors | Independent deliverable | Integration boundary |
|---|---|---|---|
| Raw inflater stream | `00bbc060`, `00bbc140`, `00bbbf00`, `00bbc1d0` | Buffered read/seek contract and typed adapter design | Existing source interface and pinned zlib; MPKG parsing stays separate |
| Single-line font layout | `00ab9fd0`, `00aba270`, `00ab6bd0`, `00ad4480` | Scalar width/advance/alignment contract before wrapping/batching | Existing FontData and glyph writer; archive loading is not a prerequisite |
| Native preload policy | `00be7ab0`, `00bde9c0`, `0073d410`, `00686380` | Identify actual startup/runtime cache selections and flags | Existing mounted-stream/FileStore interface; shared probe edits stay with primary |

The inflater and font packets now have bounded C++ implementations in disjoint
source/header files. Each worker also reviewed the other's implementation.
The primary integrated both into the existing build and D3D9 probe: MSVC Win32,
both existing CTests, a 150,123-byte buffered inflate fixture, installed font
layout/geometry, and the existing real font draw pass. See
[inflater implementation](INFLATE_STREAM_IMPLEMENTATION.md),
[font implementation](FONT_SINGLE_LINE_IMPLEMENTATION.md), and the
[integration evidence](../reports/parallel_implementation_validation.json).
No native ABI or original-game equivalence is established by these checks.

The third lane completed the synchronous physical/FileStore flags contract and
pending-request front half in [VFS_PROVIDER_FLAGS.md](VFS_PROVIDER_FLAGS.md).
Exact modes 2 and 0x32 agree on those inspected providers; the host preload
interface still accepts only its previous mode. Pending dispatch uses a separate
callback visitor and literal mode 2. Neither audit implements an async loader
or identifies native font preload selection. Ready follow-ups are package entry
integration, scalar font wrapping, and the bounded preload policy adapter;
the primary owns their shared API and probe integration.

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
