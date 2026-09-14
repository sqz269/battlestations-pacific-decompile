# Battlestations Pacific reconstruction

- Use parallel subagents for independent, bounded reconstruction work when useful.
  Assign disjoint files and Ghidra address ranges; coordinate shared metadata edits
  and integration through the primary agent. Review evidence before accepting results.
  Use `config/parallel_work.json` and `docs/PARALLEL_WORK.md` for concrete packets.
  Partition waves are candidate-graph hints, not proof of independent work; include
  named-but-incomplete dependencies and use explicit function/file ownership.
  For the continuing reconstruction goal, use the available four-agent capacity:
  one primary integrator plus up to three workers on ready independent packets.
  Refill completed worker slots when useful work is ready. Use the ledger's current
  packet states to advance archive streams/entries, font layout and VFS loading
  independently where their contracts permit. Recheck actual concurrency
  limits when resuming; do not assume whole partition segments are independent.
- Spawn workers on `gpt-5.6-sol` with `fork_turns: "none"` and a self-contained brief: the packet,
  its addresses and files, and the contract. Keep the orchestrator and integrator on `gpt-6-astra`.
  Spawn on `gpt-6-astra` instead when the packet's evidence is x87 arithmetic, register-ABI recovery,
  control-flow or listing repair, or a body too large to decompile. A full-history fork inherits the
  parent's model and cannot override it, so `fork_turns` must be `"none"` or a positive integer string
  whenever `model` is set. Set `reasoning_effort` to `"xhigh"` on every spawn that sets `model`: the
  parent's effort is not inherited and `gpt-5.6-sol` falls back to `low`.

- Use the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
  Verify project and program before every analysis/export batch. Do not re-import into `wows`.
- `config/target.json` contains local defaults; use `--config` with a file under `local/` for overrides.
- Preserve the original game installation and saved analysis. This repository builds into `build/`.
- Raw Ghidra pseudocode goes in ignored `exports/`; reconstructed C++ goes in `src/`.
  Do not compile pseudocode by inventing globals/types or stubbing unresolved calls just to link.
- Address, evidence, original ABI, and uncertainty must accompany every reconstruction.
  Check assembly when pseudocode has register inputs, x87 expressions, overlapping globals,
  or incorrect no-return annotations. Descriptive C++ names are hypotheses, not recovered symbols.
- As behavior is established, rename the corresponding Ghidra functions and add evidence comments.
  Keep an address-to-name ledger in the repository, preserve existing comments, record old values
  before edits, and save the project. Mark uncertain interpretations as provisional; retain
  correct library names. Refresh affected exports after annotation changes.
- Ledgers are sharded JSON Lines under `config/names/`, `config/reconstruction/` and `config/tags/`
  (one 64 KB address band per file; see `docs/LEDGER_INDEX.md`). Never read a ledger, the tag
  shards or the docs directory whole: query `python tools/bsp.py lookup <address>` (also `range`,
  `callers`, `callees`, `docs-for`, `segment`, `find`, all capped by `--limit`) and rebuild the
  index with `python tools/bsp.py index --if-stale` after snapshots or ledger edits. Add records with
  `python tools/bsp.py ledger add-name|add-function|add-fragment`. If a legacy monolithic
  `config/*.json` ledger exists, run `python tools/bsp.py ledger migrate` before committing.
- Context discipline. Output size is what costs; call count is nearly free.
  - Start a turn with `python tools/bsp.py brief` (one call: state, dirty files, ready packets). Do not
    re-read AGENTS.md, ROADMAP or docs for orientation; AGENTS.md is injected automatically.
  - `bsp.py` caps its own output at 2000 tokens, writes the whole result to `local/output/` and prints
    the path. When you see that marker, page the spilled file with `rg -n` or `sed -n`. Do not re-run
    with `--full` to get the rest, and never draw a conclusion from the head alone.
  - Size, not path, is the rule for reading: no single command may return more than roughly 2000 tokens.
    That applies to a header or a `.cpp` as much as to a ledger or an export. Use `--lines`, `--limit`,
    `-TotalCount`, `Select-Object -Skip N -First M`, or `rg -n -C` on a known term.
  - Batch independent reads, greps and status checks into one shell call. Keep any single command that
    can exceed roughly 4 KB in its own call, because a cap applies to the concatenated result. Commands
    that consume the previous command's output cannot be batched with it.
  - Never re-read a path already in this context. If you need a different part of it, page to that part.
  - Read code through `python tools/bsp.py show <address>` (`--asm` for the listing, `--start` to page).
    Take one representation at a time; open the assembly only when the pseudocode shows register inputs,
    x87, overlapping globals or a suspect no-return.
  - Live Ghidra questions go through `python tools/bsp.py ghidra count|proto|xrefs|callers|callees|bytes|comments|
    decompile|disasm|export` instead of inline Python; when a query repeats twice, add a subcommand.
    If nothing answers (Ghidra closed, commonly after a reboot), `python tools/bsp.py ghidra ensure`
    launches it with the project and program restored; live queries do that themselves unless
    `BSP_GHIDRA_AUTOSTART=0`.
  - `python tools/bsp.py cheatsheet` is the argument reference. Do not spend a call on `--help`.
  - Take snapshots with `python tools/bsp.py snapshot` (skips unless Ghidra's function count changed).
  - Commit in batches and push once per batch, not per commit.
  - Workers: size a packet to fill a whole turn, start it with `brief` plus the packet, and wait for
    completion notices rather than polling `wait_agent`. Prefer a fresh thread with a short handoff
    over working near the context window limit.
- Multi-harness coordination (see `docs/COORDINATION.md`; reading a named doc you are pointed to is
  always fine, the rule above is against bulk reads): each separate harness agent works in its own
  git worktree (`python tools/bsp.py worktree add <name>`, branch `agent/<name>`); the integrator
  stays on `main` and merges. Before touching addresses or output files, claim a lease: from a packet
  with `python tools/bsp.py lease claim --packet <id> --from-packet`, or ad hoc with
  `--packet <name> --addresses <a> <b> --files <paths> [--ttl hours]`; release it when done; pick work
  from `python tools/bsp.py packets ready`. Ledger commands and `ghidra_annotate.py --apply` refuse
  addresses leased to another owner. Every Ghidra mutation goes through the write lock (taken by the
  annotate/tag tools, or `coordination.ghidra_lock` around any other write); never write to Ghidra
  from inline scripts without it. Never `git add -A` in a shared checkout; stage only owned files,
  which include the ledger shards your `ledger add-*` calls created or changed (a new shard shows up
  untracked). Write multi-line commit messages to a file and use `git commit -F <file>`.
  Exports are shared through `tools/workspace.py` (the main checkout's `exports/bsp`); never create
  a junction or symlink to them inside a worktree, and remove worktrees only with
  `python tools/bsp.py worktree remove <name>` (git's own remove/clean traverse reparse points).
- Target MSVC Win32. Run `./scripts/build.ps1` after C++ changes.
  Native differential tests are enabled after `python tools/ghidra_export.py verify-seeds`.
- Ad hoc probe executables (anything compiled with `cl` outside the CMake build) must link with
  `/link /MANIFEST:EMBED` and must not have `install`, `setup`, `update` or `patch` in the file
  name. Windows UAC installer detection treats an unmanifested 32-bit exe with such a name as an
  installer and demands elevation, which blocks unattended runs and any `subprocess` capture.
  `probe` on its own is a safe name.
- Write as few new test cases as possible. Default to adding no tests for routine changes;
  use existing checks, compilation, and focused evidence inspection first. Add only the smallest
  test needed for a concrete behavioral risk or regression, or when the user explicitly asks.
  Avoid tests that mirror the implementation, duplicate existing coverage, or expand into broad
  suites and test frameworks. Prefer one focused case over many similar cases. Preserve existing
  tests unless the task calls for changing them, and run the relevant existing checks.
- Distinguish exported, reconstructed, build-tested, fixture-tested, ABI-compatible, and game-validated.
  Current math routines expose new C++ interfaces and are not drop-in binary replacements.
- Follow the user's manually configured Chrome DevTools MCP preference for any browser work.
