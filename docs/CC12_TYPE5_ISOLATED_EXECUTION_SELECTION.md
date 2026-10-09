# Type5 isolated execution selection

The first isolated Type5 selection is retained as a failed **Source=0**
attempt. Its preparation process closed with exit code 1, without a timeout,
at the complete guard's exact-family check. No native query, compiler call,
original PE read, provider call or fixture target execution followed.

The checkout is `agent/cc12_type5_fixture_execution` at
`7e4d26f1b550d623d1cba8304fbefd1dad0f798d`. The six existing Source files
matched both the accepted contract and Main. The candidate preserved the
recipe, helper and probe bytes and the existing rejection checks.

Root froze and checked 12,051 current/frozen file pairs, 14,512 membership
records and 51 scopes. Three real, closed inspectors observed the selected
14-key external and 16-key root/effective process profiles. Root's pure-text
review and materialization checks passed. These checks establish the captured
inputs and actual environments; they did not establish consistent relocation
of all consumers.

The concrete selection is `local/type5select02/current_dependency_selection.json`,
10,722,896 bytes, SHA-256
`7f0ddecb4cf51286c63a4ebeffe5d38ed616c1d14b9e00625656f69da92e4b5d`.
Materialization created exactly 16 files and closed with exit code 0. The
external preparation host subsequently passed its input/environment checks
and closed its child with exit code 1. The child's error was
`Exact fresh Root family` before the recipe's preparation function entered.

The passive peer and actual failure identify three independent defects:

- `complete_guard.py` still binds `EXPECTED_FAMILY` to Main's `local/type5p1`.
- The selected first include root and both 16-key profiles still bind Main's
  include directory. The guarded inventory contains the isolated checkout's
  2,024 include files, with zero Main include files; the recipe derives its
  includes from the isolated checkout.
- `guarded_tool_entry.py` still binds the BSP executable path to Main's
  `tools/bsp.py`, while the dispatch driver and 46 tool mappings select the
  isolated checkout's tools.

All dependent stages are held. Preserving Python bytes during relocation was
insufficient: the next fresh candidate must change the two Python path
constants, select coherent include roots and process profiles, and rebind the
PowerShell family path together. Historical audit and prior fixture paths
remain evidence inputs. Every rejection check and the original recipe,
helper, probe, Source and ABI contracts remain required.

The consumed families are sealed and must not be edited or replayed:

| Family | Exact files | Seal bytes | Seal SHA-256 |
| --- | ---: | ---: | --- |
| `local/type5select02` | 12,107 | 3,439,684 | `d413f88b44b261b03ab00fbddfab23828e86dbb8a059e97447bbf5c6038b2fa9` |
| `local/type5p2` | 29 | 7,566 | `b5bf4d12050c6de9d12e5bca7a61f43b4847fa7d566df0ea21dedf164bb365d3` |

Root independently reverified every listed artifact and exact recursive file
membership after sealing. Details are in
`reports/cc12_type5_isolated_execution_selection.json` and the ignored
`local/type5p2_failed_seal_closure01.json`. No C++ changes, new tests, Source
admission, ABI qualification, game validation or prior runtime replay is claimed.
The continuing reconstruction goal remains active.
