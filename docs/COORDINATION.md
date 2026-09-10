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

## Leases

A lease says who is working on which addresses, ranges and output files. The registry lives
outside every worktree (`coordination_dir` in config/target.json, default `~/.bsp/leases.jsonl`)
so all checkouts see the same state immediately, without waiting for a merge.

```powershell
python tools/bsp.py lease claim --packet <id> --from-packet            # addresses, ranges, files from parallel_work.json
python tools/bsp.py lease claim --packet ad_hoc --addresses 00bea680 00bf0430 --files src/x.cpp --ttl 4
python tools/bsp.py lease list | check <address> | release [--packet <id>] | lock-status
```

- A claim is refused when it overlaps another owner's active lease (addresses, ranges or
  files). Claiming the same packet again extends your own lease.
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
