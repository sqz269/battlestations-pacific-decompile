# Claude Code entry point

The project rules live in AGENTS.md so that Codex and Claude read the same text. Everything
below is imported from it; edit AGENTS.md, not this file.

@AGENTS.md

Claude-specific notes:
- Start every turn with `python tools/bsp.py state`; it shows your owner name (the git branch of
  this checkout), your lease, and the index freshness.
- If you are a spawned worker, you should be in your own worktree on an `agent/<name>` branch
  with a lease claimed for your packet (`docs/COORDINATION.md`). If `state` shows you on `main`
  in the main checkout, you are in the integrator's tree: do not edit or stage anything there.
