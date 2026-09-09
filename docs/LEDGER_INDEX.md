# Sharded ledgers and the lookup index

Addresses: none (workspace tooling)

## Why

A worker agent must be able to ask "what is known about `00ab9fd0`" without loading a
multi-megabyte ledger or scanning 140 docs into its context, and several agents must be
able to record findings at the same time without merge conflicts. The reviewed ledgers
therefore live as JSON Lines shards keyed by 64 KB address band, and a disposable SQLite
index answers cross-reference questions through a CLI with capped output.

## Layout

| Path | Content | Written by |
| --- | --- | --- |
| `config/names/<band>.jsonl` | reviewed descriptive names: `address`, `name`, `evidence` | `bsp.py ledger add-name`, or one appended line |
| `config/reconstruction/<band>.jsonl` | `kind` (`function`/`fragment`), `address`, `name`, `source`, `status`, ... | `bsp.py ledger add-function` / `add-fragment` |
| `config/reconstruction/meta.json` | `game_rebuilt` flag | hand |
| `config/tags/<band>.jsonl` + `evidence.json` | generated inventory tags; rows reference an `evidence_id` | `build_tag_ledger.py` |
| `local/bsp_index.sqlite` | derived index, ignored, rebuilt by `bsp.py index` | tool |

`<band>` is the address with the low 16 bits cleared, e.g. `00ab0000.jsonl`. A record is one
line; shards are sorted by address when a tool rewrites them, and a plain appended line is
also accepted. Two agents editing different bands never conflict; edits to the same band
merge line by line.

The legacy monolithic files (`config/ghidra_names.json`, `config/reconstruction.json`,
`config/ghidra_tags.json`) are still read if they exist, and a legacy record overrides a
shard record because it is the newer in-flight edit. `python tools/bsp.py ledger migrate`
moves them into shards and removes them; run it before committing if a legacy file exists.

## Rules for agents

- Start a turn with `python tools/bsp.py state`: snapshot, ledger counts, index freshness,
  git summary and the current work packets on one screen. It replaces re-reading AGENTS.md,
  ROADMAP and docs for orientation.
- Never read a ledger, `config/tags/`, an export, or the docs directory whole. Ask the index:
  `python tools/bsp.py lookup <address>` prints one card (name, segment, tag, reviewed name
  and evidence, reconstruction records, callers, callees, strings, docs), and
  `python tools/bsp.py show <address> [--asm] [--lines N] [--start K]` prints a capped excerpt
  of the exported pseudocode or listing with its callees named (`--live` fetches an unsaved
  view when nothing is exported).
- Live Ghidra questions go through `python tools/bsp.py ghidra count|proto|xrefs|callers|
  callees|bytes|comments|decompile|disasm|export ...` (capped, verified project) instead of inline Python.
  For annotation readback, `ghidra comments <addresses...> --output local/comments.json`
  stores full records in ignored storage and prints only their count and path.
- `range`, `callers`, `callees`, `docs-for`, `segment` and `find` are capped by `--limit`;
  raise it deliberately rather than dumping everything.
- `python tools/bsp.py snapshot` takes a snapshot and rebuilds the index only when Ghidra's
  function count changed; `python tools/bsp.py index --if-stale` rebuilds after ledger, sweep,
  partition or doc changes (a digest of the inputs decides; nothing is authored in the index).
- New docs get an `Addresses:` line under the title listing the function starts they
  cover, so `docs-for` finds them even when the prose does not repeat every address.
- Ghidra plate comments remain the copy of the evidence a model sees while decompiling;
  the ledger line points to the doc and carries one line of evidence, not a third copy.

## Index contents

`functions` (snapshot names, export presence, segment, tag, reviewed name, reconstruction
record), `calls` and `datarefs` from `tools/callgraph_sweep.py` (direct calls only; string
text resolved from the disk PE), `segments` from `reports/library_inventory/candidate_partition.json`,
and `docs`/`doc_addresses` from address mentions in `docs/*.md`.
