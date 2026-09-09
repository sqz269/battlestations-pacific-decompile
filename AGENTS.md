# Battlestations Pacific reconstruction

- Use parallel subagents for independent, bounded reconstruction work when useful.
  Assign disjoint files and Ghidra address ranges; coordinate shared metadata edits
  and integration through the primary agent. Review evidence before accepting results.

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
- Target MSVC Win32. Run `./scripts/build.ps1` after C++ changes.
  Native differential tests are enabled after `python tools/ghidra_export.py verify-seeds`.
- Write as few new test cases as possible. Default to adding no tests for routine changes;
  use existing checks, compilation, and focused evidence inspection first. Add only the smallest
  test needed for a concrete behavioral risk or regression, or when the user explicitly asks.
  Avoid tests that mirror the implementation, duplicate existing coverage, or expand into broad
  suites and test frameworks. Prefer one focused case over many similar cases. Preserve existing
  tests unless the task calls for changing them, and run the relevant existing checks.
- Distinguish exported, reconstructed, build-tested, fixture-tested, ABI-compatible, and game-validated.
  Current math routines expose new C++ interfaces and are not drop-in binary replacements.
- Follow the user's manually configured Chrome DevTools MCP preference for any browser work.
