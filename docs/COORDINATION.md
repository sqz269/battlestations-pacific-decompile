# Worktrees, leases and the Ghidra write lock

Addresses: none (workspace tooling)

Several harness agents (the Codex integrator and its workers, Claude sessions, anything
else) can work on the reconstruction at once. Three things are shared and need rules: the
git history, the single Ghidra project, and the address space itself. Everything else
(exports, index, build directory, scratch) is per function or per checkout and needs nothing.

## One worktree per harness agent

```powershell
python tools/bsp.py worktree add <name> [--packet <id>]   # ../battlestations-pacific-decompile-<name> on branch agent/<name>
python tools/bsp.py worktree list
```

Exports (snapshot, call graph, per-function pseudocode) are shared, not copied: every tool
resolves them to the MAIN checkout's `exports/bsp` through `git rev-parse --git-common-dir`
(`tools/workspace.py`; `BSP_EXPORTS_DIR` overrides). Never create a junction or symlink to
`exports` inside a worktree: `git worktree remove` and `git clean` traverse reparse points on
Windows and empty the shared target, which happened once. Remove worktrees with
`python tools/bsp.py worktree remove <name> [--delete-branch]`, which detaches any legacy
junction before calling git. Build directories and `local/` (index, logs) are per worktree.
The agent's owner name defaults to its branch (`agent/<name>`); set `BSP_AGENT` to override.
The integrator stays on `main` in the main checkout, merges `agent/*` branches, and owns
shared headers, CMake, probes and Ghidra mutations as before.

## Integrating worker branches

`python tools/integrate_workers.py agent/<name> [agent/<other>...] [--no-push] [--skip-build]`,
run from the main checkout, merges each branch into the `agent/integrate` worktree (create it once
with `python tools/bsp.py worktree add integrate`), resolves the mechanical conflicts through
`tools/merge_resolve.py` (registry-line union for `cmake/startup.cmake`, address union for ledger
shards with the incoming side winning for records it changed, same-spot insertions and diff3
pure insertions in text files), builds and runs the tests there, fast-forwards `main` (merging
the other integrator's uncommitted shard edits by address when they block the checkout), applies
the newly added reviewed names to Ghidra under the write lock, refreshes the snapshot and index,
and pushes. It stops before touching `main` on any semantic conflict or build failure. Merge
commits take their trailer lines from `local/commit-trailer.txt`. Afterwards retire the branch
with `python tools/bsp.py worktree remove <name> --delete-branch`.

When a worker names a routine it read from the raw listing (no Ghidra function), the integrator
defines it first with `python tools/ghidra_define_function.py <start> <end_exclusive> [--record reports/<x>.json]`,
which verifies the bytes against the disk image, disassembles the range explicitly and creates the function
without re-running flow discovery, under the write lock; then it re-runs the annotate step. The write lock
is reclaimed automatically when its holder process is gone.

Ghidra's non-returning discovery marks calls to the CRT free helpers as CALL_RETURN, so the bytes after
such a call stay undisassembled and the decompiler drops the reachable block. `python tools/bsp.py ghidra flow
<function>` reports those gaps read-only (comparing the stored listing with the disk bytes; the bridge's
dry-run clear is not a query, it clears), and `python tools/ghidra_flow_repair.py <function> --apply
[--record reports/<x>.json]` clears the call-site overrides and disassembles the gaps under the write lock,
never touching the callee's own flag. Run it on a function whose pseudocode shows a spurious return after
`_free` before a worker reads it.

## Leases

A lease says who is working on which addresses, ranges and output files. The registry lives
outside every worktree (`coordination_dir` in config/target.json, default `~/.bsp/leases.jsonl`)
so all checkouts see the same state immediately, without waiting for a merge.

```powershell
python tools/bsp.py lease claim --packet <id> --from-packet            # addresses, ranges, files from parallel_work.json
python tools/bsp.py lease claim --packet ad_hoc --addresses 00bea680 00bf0430 --files src/x.cpp --ttl 4
python tools/bsp.py lease list [-v] | check <address|file>... | release [--packet <id>] | lock-status   # -v lists each lease's addresses
```

- A claim is refused when it overlaps another owner's active lease (addresses, ranges or
  files). Claiming the same packet again extends your own lease.
- Your owned files are the packet's output files plus any ledger shard your `ledger add-*`
  calls created or modified; a brand-new shard appears untracked, a modified one shows as
  changed, and both are staged by path. `ledger add-*` refuses to overwrite an existing record
  unless you pass `--replace`; the superseded line stays in git history. Write commit messages
  to a file (`local/commit-msg.txt`) and commit with `-F`; the PowerShell tool has no heredocs.
  Attribution trailers follow whatever the harness that makes the commit requires.
- All `bsp.py` read commands work from inside a worktree (exports resolve to the main checkout);
  running read-only commands from the main checkout is also fine. Segment ids come from the
  partition file of the checkout you run in, so two checkouts with different partition files
  number segments differently; cite addresses, not segment ids.
- Leases expire (default 8 hours); expired leases do not block anyone. Release when done.
- `bsp.py ledger add-name|add-function|add-fragment` and `ghidra_annotate.py --apply` refuse
  addresses leased to someone else (`--force` on the ledger commands for a deliberate override).
- `state` shows the current owner, the active leases and whether this owner holds one.

## Packets and dependencies

`config/parallel_work.json` is the packet ledger (integrator-owned, tracked in git). Two
optional fields make it a queue: `depends_on: [packet ids]` and `done: true`. A packet whose
`state` contains `integrated` also counts as done.

```powershell
python tools/bsp.py packets list      # done | leased:<owner> | blocked:<deps> | ready
python tools/bsp.py packets ready     # what a free agent may claim now
python tools/bsp.py packets depend <id> --on <dep-id> ...
python tools/bsp.py packets done <id>
```

The partition report's strong-dependency table is the seed for `depends_on`; the integrator
confirms it, because a below-threshold call or an indirect dispatch can still matter.

## Ghidra write lock

The Ghidra project is one server with no authentication, so mutations are serialized by a
lock file in the coordination directory (`ghidra.lock`, owner, pid, expiry). `ghidra_annotate.py
--apply` and `ghidra_tag.py --apply` take it automatically; `bsp.py lease lock-status` shows
it. Any other write path (function creation, comments, prototypes from inline scripts) must
wrap itself in `coordination.ghidra_lock(...)` or leave the write to the integrator. Reads
need no lock. An expired lock is reclaimed automatically; a live one is waited for up to two
minutes, then the caller fails with the holder's name.

## What this does not do

It does not stop an agent that ignores the tools; it makes the honest path the easy path.
It does not merge branches or resolve semantic conflicts; the integrator does. And it does
not make Ghidra writes parallel: with one project, writes remain one at a time by design.
