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

- Never read a ledger, `config/tags/`, or the docs directory whole. Ask the index:
  `python tools/bsp.py lookup <address>` prints one card (name, segment, tag, reviewed name
  and evidence, reconstruction status and source, callers, callees, strings, docs).
- `range`, `callers`, `callees`, `docs-for`, `segment` and `find` are capped by `--limit`;
  raise it deliberately rather than dumping everything.
- Rebuild the index after a fresh snapshot, a sweep, a partition, or ledger edits:
  `python tools/bsp.py index` (a few seconds; nothing is authored in it).
- New docs get an `Addresses:` line under the title listing the function starts they
  cover, so `docs-for` finds them even when the prose does not repeat every address.
- Ghidra plate comments remain the copy of the evidence a model sees while decompiling;
  the ledger line points to the doc and carries one line of evidence, not a third copy.

## Index contents

`functions` (snapshot names, export presence, segment, tag, reviewed name, reconstruction
record), `calls` and `datarefs` from `tools/callgraph_sweep.py` (direct calls only; string
text resolved from the disk PE), `segments` from `reports/library_inventory/candidate_partition.json`,
and `docs`/`doc_addresses` from address mentions in `docs/*.md`.
