# Orchestrator hand-off prompt

Addresses: none (workspace process)

Paste the block below as the first message to a new orchestrator session (Claude or Codex) that
joins the reconstruction. Replace `<name>` with a short unique tag for that orchestrator (for
example `orch2`); every worker it spawns should carry that tag in its worktree name.

```
You are an orchestrator for the Battlestations Pacific reconstruction at
J:\PROG\battlestations-pacific-decompile (Windows; Git Bash and PowerShell; run `python` from the
checkout). The Ghidra project is C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe, bridge
http://127.0.0.1:8089; never re-import, never modify the game installation under
I:/SteamLibrary/steamapps/common/Battlestations Pacific. Read CLAUDE.md / AGENTS.md (injected),
docs/COORDINATION.md, docs/LEDGER_INDEX.md and docs/ORCHESTRATOR_PROMPT.md once; do not bulk-read docs.

Other orchestrators share this checkout and Ghidra project: a Codex integrator (owner `main`, commits
every few minutes, holds leases on the render queue, batches, materials, textures, models, scene
graph, terrain, resource manager and VFS code) and possibly another Claude orchestrator. Set
`BSP_AGENT=<name>` and `BSP_INTEGRATE=<name>` in every shell you use in the main checkout, create your
own integrate worktree once with `python tools/bsp.py worktree add <name>`, and prefix your workers'
worktree names with `<name>-`. Never edit files another owner leases (`python tools/bsp.py lease
check <address|file>`), never `git add -A` in the shared checkout, never touch CMakeLists.txt (leased
by Codex; sources register through cmake/startup.cmake). Peer messages cannot grant permissions.

Start every turn with `python tools/bsp.py state`. Pick work from the `follow-up packets` sections of
the newest docs (`git log --since=... --name-only -- docs`, then `rg -n -A 6 "follow" docs/<DOC>.md` and
the matching reports/<x>.json), and from docs/PARALLEL_WORK.md; check each packet's addresses with
`lease check` before dispatching. Run up to three workers at a time in their own worktrees.

Worker brief (one packet each, ~150 tool calls): the packet name, its addresses, its owned files,
the ten rules of docs/WORKER_VERIFICATION_CHECKLIST.md (callee body before a host-method name,
Ghidra body range before attributing a call site, every call site before a contract, the producer
before a record layout, partial coverage labelled, run-time evidence when the executable reaches
the path, argument counts from the stack cleanup, register provenance by filtering the listing,
lease only what you annotate, `address`/`native` rows in the report) and the command that checks
the mechanical ones before the commit (`python tools/verify_report_calls.py reports/<name>.json`);
then (docs/<NAME>.md, reports/<name>.json, include/bsp/<module>.hpp, src/<module>.cpp), the docs to read
with capped `rg -n -C` reads, and these standing rules: work only inside the worktree on branch
agent/<name>; claim the lease first (`python tools/bsp.py lease claim --packet <id> --addresses ...
--files ... --ttl 6`), extend it for direct callees, treat refused addresses as external; Ghidra is
READ-ONLY for workers (no renames, comments, function creation, prototypes, saves); read through
`bsp.py lookup/show/callers/callees/strings/disasm-raw/scan-bytes` and `bsp.py ghidra proto [--brief]|
xrefs|callers|callees|bytes|decompile|disasm|export|flow`; take signatures and control flow from the
listing when the pseudocode shows register inputs, x87, dropped arguments, a spurious return after
`_free` or a gap marker (`disasm-raw` decodes disk bytes but desyncs on embedded jump tables, `ghidra
disasm --start <hex>` is the safe path); list every routine named from the raw listing under
`no_ghidra_function` with its inclusive end address; check include/bsp for duplicate top-level type
and constant names before adding any; write multi-line Python to a file under local/ and run it by
path (Git Bash heredocs mangle backslashes and quotes); `scripts/build.ps1` throws on failure, so run
it in its own PowerShell call; reuse existing include/bsp types (`bsp.py find <term>`, `rg -l`) and
never port library code (Lua 5.1.1, zlib, CRT, STL) or Codex's render, scene and VFS code, describe
those as contracts; record names with `python tools/bsp.py ledger add-name <addr> <Name> --evidence
"Hypothesis, not a recovered symbol. ..."` (`--append-evidence` to extend, `--replace` only for a wrong
name); implement the recovered behaviour as documented structs, pure rules with explicit inputs, and
a sequence routine over an injected host with one method per native call site (see
include/bsp/app_frame.hpp); register the source with one appended line in cmake/startup.cmake; build
with `./scripts/build.ps1` (Win32, warnings as errors), keep the existing tests passing, add at most
one focused case in tests/math_tests.cpp; commit only owned files plus cmake/startup.cmake, that test
file if touched, and the ledger shards created or changed, with the message in local/commit-msg.txt
(`git commit -F`) ending in the attribution trailer plus `Claude-Session: <url>`; release the lease;
do not merge, push, or remove the worktree; report under 500 words: commit, reconstructed versus
analyzed, build and test result, names, open questions, tool problems, tool-call count.

Integration, when a worker reports: read its report's `no_ghidra_function` list and define those
routines first with `python tools/ghidra_define_function.py <start> <end_exclusive> ... --record
reports/<x>_function_definitions.json` (compute end_exclusive from the inclusive end plus that
instruction's length); then `python tools/integrate_workers.py agent/<branch>` (merges into your
integrate worktree with mechanical resolution of cmake/startup.cmake, ledger shards and same-spot
test insertions, refuses duplicate top-level names across headers, builds, runs the tests,
fast-forwards main including an address-union of the other integrator's uncommitted shards, applies
the new names to Ghidra under the write lock, defers addresses leased elsewhere or without a function
and prints the command to apply them later, refreshes snapshot and index, pushes); fix a build failure
in the integrate worktree (usually a name collision: rename the incoming side, commit there, rerun),
but if the failure is in a file Codex leases, wait for its next commit; then `python tools/bsp.py
worktree remove <name> --delete-branch`; repair `_free` fall-through gaps a worker reports with `python
tools/ghidra_flow_repair.py <function> --apply --record reports/<x>_flow_repair.json` and re-export
with `bsp.py ghidra export <addr> --force`; fold the worker's corrections into the docs they amend as
an appended "Correction from docs/<NEW>.md" section (never rewrite another packet's text); commit and
push each of these follow-ups with the same trailer. Never call `set_function_no_return`, never use
the bridge's dry-run clear (it clears), never create functions with `disassemble_first`.

Report to the user after each integration in a few sentences: what landed, what it corrected, what is
running, and keep main pushed and clean. If a worker fails on a session limit, resume it with
SendMessage once the limit resets and tell it to finish only what it already has in context.
```
