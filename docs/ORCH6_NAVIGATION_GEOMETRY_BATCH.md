# Navigation geometry reconstruction batch

Addresses: 00417610 0041A200 0041AEA0 0041B840 004F3560 004F3970 004F4520 004F47B0 009D58F0 009D6550 009D68B0

Orchestrator `orch6-20260912` works from its own `agent/orch6-20260912`
worktree. The three worker packets extend the recent ship path-planner and
path-follower handoffs. Addresses and files are leased independently;
workers use read-only Ghidra queries against project `bsp`, program
`/battlestationspacific.exe`. The installation and original executable are
read-only inputs. Descriptive names remain hypotheses, not recovered symbols.

| Packet | Deliverable | Evidence |
| --- | --- | --- |
| Lateral records | Producer-backed 36-byte (0x24) record fields, wrapped indexing and derived polygon directions/winding | `docs/SHIP_AI_LATERAL_RECORD.md`, `reports/ship_ai_lateral_record.json` |
| Circle geometry | Circle intersections, concrete tangent dependencies and corrections to existing tangent projections | `docs/SHIP_AI_NAV_CIRCLE_TANGENT.md`, `reports/ship_ai_nav_circle_tangent.json` |
| Nearest boundary | Distance/direction and nearest-boundary control flow over explicit host contracts | `docs/AVOID_ZONE_BOUNDARY.md`, `reports/avoid_zone_boundary.json` |

The initially selected segment-intersection packet was claimed concurrently
by `agent/cc-ai-avoid-zones`. The lease tool refused the overlapping claim
before any edits. The third worker instead claimed `0041AEA0`/`0041B840`
and distinct boundary files. Its retained worktree name is
`orch6-avoid-geometry`; its completed packet is `orch6_avoid_zone_boundary`.

## Independent review

The integrator checked these claims against the live listing, separately
from the worker reports:

- `009D5920` is an interior field store, not a function entry. Its containing
  function is `009D58F0-009D5929`; `00417610` is called at `009D5917`.
- `009D68B7` compares the low byte of the side argument. `009D6A16` and
  `009D6A1E` round products before the center additions.
- `004F3A36`/`004F3A3B` continue on an unordered gap comparison. The former
  C++ rejection condition inverted that NaN behavior.
- The intersection caller reuses the chosen-point slots for its first
  output. Its second output slots have no initialization on a no-write
  intersection result. An explicit seed in the new C++ interface models
  caller-provided bits; its compatibility default is a defined deviation.
- `0041AEA0` computes the outside direction as query minus closest point;
  `0041B840` subtracts direction times distance plus push. A positive push
  continues toward and past the closest boundary, rather than adding
  outside clearance. Nonpositive best distances return the original query.

The original handoff documents have appended correction sections pointing
to the detailed packet evidence. Their earlier text is preserved.

## Validation and limits

`reports/orch6_navigation_geometry_batch.json` records the reviewed commits,
mechanical call-site checks, Win32 build/CTest results, focused probe results,
annotation/export status and integration revision. Existing native seed-byte
verification was run in the integrator worktree to enable its existing native
math checks; it verifies only the seed ranges, not whole-image identity.

These are reconstructed interfaces and bounded host compositions. They are
not drop-in object-layout or original-ABI replacements. The boundary host
still requires the documented callee bindings. The inspected executable
host did not wire these boundary routines into its placeholder zone queries.
Build and fixture evidence therefore do not establish in-game avoidance,
whole-program equivalence, or complete floating-point identity.
